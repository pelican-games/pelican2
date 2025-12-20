#pragma once

#include "coretemplate.hpp"
#include "coretemplate.hpp"
#include "../../../profiler.hpp"
#include "../../../job_system.hpp"
#include "../../../container.hpp"
#include "../../../ecs/componentinfo.hpp"

#include <queue>
#include <algorithm>

namespace Pelican {

namespace internal {
    size_t getIndexFromComponentId_Ref(ComponentId id) {
        auto& mgr = GET_MODULE(ComponentInfoManager);
        return mgr.getIndexFromComponentId(id);
    }
}

void ECSCoreTemplatePublic::updateSystemChunkCache(ChunkIndex chunk_index) {
    auto &chunk = chunks_storage[chunk_index];
    for (auto &[id, sys] : systems) {
        if ((chunk.getMask() & sys.matching_mask) == sys.matching_mask) {
            sys.matching_chunk_indices.push_back(chunk_index);
        }
    }
}

    EntityId ECSCoreTemplatePublic::allocateEntity(std::span<const ComponentId> component_ids, std::span<void *> component_ptrs,
                                 size_t count) {
    TimeProfilerStart("ECS_AllocateEntity");
    // Entity id is recorded as implicit component
    ComponentId component_ids_ex[65]; // MAX_COMPONENTS + 1
    size_t ex_size = component_ids.size() + 1;
    if(ex_size > 65) { return 1; }
    component_ids_ex[0] = ComponentIdByType<EntityId>::value;
    for(size_t i=0; i<component_ids.size();++i) component_ids_ex[i+1] = component_ids[i];

    // Convert to Dense Indices
    auto& mgr = GET_MODULE(ComponentInfoManager);
    std::vector<size_t> component_indices_ex(ex_size);
    for(size_t i=0; i<ex_size; ++i) {
        component_indices_ex[i] = mgr.getIndexFromComponentId(component_ids_ex[i]);
    }

    // Sort component IDs to form the archetype key
    std::vector<ComponentId> archetype_key(component_ids_ex, component_ids_ex + ex_size);
    std::sort(archetype_key.begin(), archetype_key.end());
    
    // find suitable chunk using archetype index
    ChunkIndex chunk_index = UINT32_MAX;
    
    auto it = archetype_to_chunks.find(archetype_key);
    if (it != archetype_to_chunks.end()) {
        for (auto idx : it->second) {
            if (chunks_storage[idx].size() + count <= ECSComponentChunk::CHUNK_CAPACITY) {
                chunk_index = idx;
                break; 
            }
        }
    }

    const auto entity_id_comp_idx = component_indices_ex[0];

    // insert entity
    EntityId entity_id_first = 0; 

    // add new chunk if suitable chunk is not found
    if (chunk_index == UINT32_MAX) {
        chunk_index = chunks_storage.size();
        // Construct with INDICES and GENERIC IDs
        chunks_storage.emplace_back(std::span(component_indices_ex), std::span(component_ids_ex, ex_size));
        
        // Register in archetype map
        archetype_to_chunks[archetype_key].push_back(chunk_index);
        
        // Update system cache
        updateSystemChunkCache(chunk_index);
    }

    const auto first_index = chunks_storage[chunk_index].size();

    std::vector<void *> component_ptrs_ex(component_ids.size() + 1);
    
    const auto allocated_count = chunks_storage[chunk_index].allocate(component_indices_ex, component_ptrs_ex, count);
    
    // Set version of all components in this chunk to global_tick
    for(auto idx : component_indices_ex) {
         chunks_storage[chunk_index].updateVersion(idx, global_tick);
    }

    std::copy(component_ptrs_ex.begin() + 1, component_ptrs_ex.end(), component_ptrs.begin());

    EntityId *entity_ids = static_cast<EntityId *>(chunks_storage[chunk_index].getRef(entity_id_comp_idx).ptr);

    for (size_t i = 0; i < count; i++) {
        EntityRef entity_ref{
            .chunk_index = chunk_index,
            .array_index = static_cast<WithinChunkIndex>(first_index + i),
        };

        uint32_t my_idx = 0;
        uint32_t my_gen = 0;

        // Try to recycle first
        if (!free_indices.empty()) {
             my_idx = free_indices.back();
             free_indices.pop_back();
             id_to_ref[my_idx] = entity_ref;
             my_gen = entity_generations[my_idx];
        } else {
             // Append new
             my_idx = id_to_ref.size();
             id_to_ref.emplace_back(entity_ref);
             // Ensure generation vector matches size
             if (entity_generations.size() <= my_idx) {
                 entity_generations.resize(my_idx + 1, 0);
             }
             my_gen = 0; 
        }

        EntityId my_id = (static_cast<EntityId>(my_gen) << 32) | my_idx;
        entity_ids[i] = my_id;
        
        if (i == 0) entity_id_first = my_id;
    }

    TimeProfilerEnd("ECS_AllocateEntity");
    return entity_id_first;
}

void ECSCoreTemplatePublic::remove(EntityId id) {
    uint32_t index = static_cast<uint32_t>(id & 0xFFFFFFFF);
    uint32_t gen = static_cast<uint32_t>(id >> 32);

    if (index >= entity_generations.size() || entity_generations[index] != gen) {
        LOG_WARNING(logger, "Attempt to remove stale or invalid EntityId: {:x} (Index: {}, Gen: {})", id, index, gen);
        return;
    }
    entity_generations[index]++; 
    free_indices.push_back(index);

    const auto ref = id_to_ref[index];
    auto &chunk = chunks_storage[ref.chunk_index];

    // Get EntityId component array using Dense Index
    auto& mgr = GET_MODULE(ComponentInfoManager);
    size_t entity_id_idx = mgr.getIndexFromComponentId(ComponentIdByType<EntityId>::value);

    EntityId moved_id = static_cast<EntityId *>(chunk.getRef(entity_id_idx).ptr)[chunk.size() - 1];

    // Swap and erase
    for (const auto index : chunk.getIndices()) {
        auto component_arr = chunk.getRef(index);

        auto ptr = static_cast<uint8_t *>(component_arr.ptr);
        auto stride = component_arr.stride;
        std::memcpy(ptr + stride * ref.array_index, ptr + stride * (chunk.size() - 1), stride);
    }

    chunks_storage[ref.chunk_index].free(1);
    uint32_t moved_index = static_cast<uint32_t>(moved_id & 0xFFFFFFFF);
    
    if (moved_index < id_to_ref.size()) {
         id_to_ref[moved_index].array_index = ref.array_index;
    } else {
        LOG_ERROR(logger, "Moved entity has invalid index: {}", moved_index);
    }
}

void ECSCoreTemplatePublic::compaction() {
    TimeProfilerStart("ECS_Compaction");
    
    // Get EntityId component array using Dense Index
    auto& mgr = GET_MODULE(ComponentInfoManager);
    size_t entity_id_idx = mgr.getIndexFromComponentId(ComponentIdByType<EntityId>::value);

    for (auto &[archetype, chunk_indices] : archetype_to_chunks) {
        if (chunk_indices.empty()) continue;

        size_t target_chunk_list_idx = 0;
        size_t source_chunk_list_idx = chunk_indices.size() - 1;

        while (target_chunk_list_idx < source_chunk_list_idx) {
            ChunkIndex target_idx = chunk_indices[target_chunk_list_idx];
            ChunkIndex source_idx = chunk_indices[source_chunk_list_idx];
            
            auto& target_chunk = chunks_storage[target_idx];
            auto& source_chunk = chunks_storage[source_idx];

            // If target is full, move next
            if (target_chunk.size() >= ECSComponentChunk::CHUNK_CAPACITY) {
                target_chunk_list_idx++;
                continue;
            }

            // If source is empty, move prev
            if (source_chunk.size() == 0) {
                source_chunk_list_idx--;
                continue;
            }

            // Calculate how many to move
            size_t available_space = ECSComponentChunk::CHUNK_CAPACITY - target_chunk.size();
            size_t source_count = source_chunk.size();
            size_t move_count = std::min(available_space, source_count);
            
            // Allocate space in target
            size_t target_start_index = target_chunk.size();
            auto indices = target_chunk.getIndices();
            std::vector<void*> dummy_ptrs(indices.size() + 1);
            std::vector<size_t> indices_vec(indices.begin(), indices.end());
            std::vector<void*> ptrs_vec(indices.size()); 
            
            target_chunk.allocate(indices_vec, ptrs_vec, move_count);
            
            // Now Copy Data
            for (size_t i = 0; i < move_count; ++i) {
                size_t src_index_in_chunk = source_chunk.size() - 1 - i;
                size_t dst_index_in_chunk = target_start_index + i;
                
                // For each component type (by index)
                for (auto comp_idx : indices) {
                    auto src_ref = source_chunk.getRef(comp_idx);
                    auto dst_ref = target_chunk.getRef(comp_idx);
                    
                    std::memcpy(
                        static_cast<uint8_t*>(dst_ref.ptr) + dst_ref.stride * dst_index_in_chunk,
                        static_cast<uint8_t*>(src_ref.ptr) + src_ref.stride * src_index_in_chunk,
                        src_ref.stride
                    );
                    
                    // Update version
                    target_chunk.updateVersion(comp_idx, global_tick);
                }
                
                // Update EntityRef
                EntityId* entity_ids_ptr = static_cast<EntityId*>(source_chunk.getRef(entity_id_idx).ptr);
                EntityId moved_entity_id = entity_ids_ptr[src_index_in_chunk];
                
                id_to_ref[moved_entity_id].chunk_index = target_idx;
                id_to_ref[moved_entity_id].array_index = dst_index_in_chunk;
            }

            // Remove from source
            source_chunk.free(move_count);
        }
        
        // After compaction of this archetype, check for minimized chunks
        size_t minimized_count = 0;
        for (auto idx : chunk_indices) {
            auto& chunk = chunks_storage[idx];
            if (chunk.size() == 0) {
                 chunk.minimize();
                 minimized_count++;
            }
        }
        if (minimized_count > 0) {
            LOG_INFO(logger, "Compaction: Archetype (size {}) - Minimized {} empty chunks", archetype.size(), minimized_count);
        }
    }

    TimeProfilerEnd("ECS_Compaction");
}

size_t ECSCoreTemplatePublic::getTotalCapacity() const {
    size_t total = 0;
    for(const auto& chunk : chunks_storage) {
        total += chunk.getCapacityBytes();
    }
    return total;
}

void ECSCoreTemplatePublic::getContiguousEntityIds(EntityId start_id, size_t count, std::vector<EntityId>& out_ids) {
    uint32_t index = static_cast<uint32_t>(start_id & 0xFFFFFFFF);
    if(index >= id_to_ref.size()) return;

    const auto ref = id_to_ref[index];
    auto& chunk = chunks_storage[ref.chunk_index];
    
    // Get EntityId array
    auto& mgr = GET_MODULE(ComponentInfoManager);
    size_t entity_id_idx = mgr.getIndexFromComponentId(ComponentIdByType<EntityId>::value);
    
    EntityId* ptr = static_cast<EntityId*>(chunk.getRef(entity_id_idx).ptr);
    
    // Safety check
    if (ref.array_index + count > chunk.size()) {
        // This should not happen if allocated contiguously and not removed yet
        return; 
    }

    for(size_t i=0; i<count; ++i) {
        out_ids.push_back(ptr[ref.array_index + i]);
    }
}

void ECSCoreTemplatePublic::unregisterSystem(SystemId system_id) {
    for (const auto depends : systems.at(system_id).depends_list) {
        systems.at(depends).depended_by.erase(system_id);
    }
    systems.erase(system_id);
}

void ECSCoreTemplatePublic::update() {
    global_tick++; 
    JobSystem::Get().init(); 

    TimeProfilerStart("ECS_Update_Sort");
    
    // Level-based Topological Sort
    std::unordered_map<SystemId, size_t> in_degree;
    std::queue<SystemId> zero_degree_queue; 
    
    for (const auto &[id, sys] : systems) {
        size_t d = sys.depends_list.size();
        in_degree[id] = d;
        if (d == 0) zero_degree_queue.push(id);
    }
    
    std::vector<std::vector<SystemId>> execution_levels;
    
    while (!zero_degree_queue.empty()) {
        std::vector<SystemId> current_level;
        size_t level_size = zero_degree_queue.size();
        for(size_t i = 0; i < level_size; ++i) {
            SystemId id = zero_degree_queue.front();
            zero_degree_queue.pop();
            current_level.push_back(id);
        }
        
        execution_levels.push_back(std::move(current_level));
        
        for (const auto& executed_id : execution_levels.back()) {
            for (const auto depended : systems.at(executed_id).depended_by) {
                in_degree[depended]--;
                if (in_degree[depended] == 0) {
                    zero_degree_queue.push(depended);
                }
            }
        }
    }

    TimeProfilerEnd("ECS_Update_Sort");

    TimeProfilerStart("ECS_Update_Execution");

    // Execute levels
    for (const auto& level : execution_levels) {
        if (level.empty()) continue;
        
        for (const auto& sys_id : level) {
             JobSystem::Get().schedule([this, sys_id]() {
                auto &sys = systems.at(sys_id);
                // Pass component_indices to p_func
                sys.p_func(*this, sys.system_ref, sys.matching_chunk_indices, sys.component_indices);
             });
        }
        
        JobSystem::Get().wait();
    }
    
    TimeProfilerEnd("ECS_Update_Execution");
}

} // namespace Pelican