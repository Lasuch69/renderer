#include <rendering/device/rendering_device.h>
#include <rendering/storage/mesh_storage.h>

#include "renderer_scene_draw.h"

void RSD::draw() {
	MeshStorage &mesh_storage = MeshStorage::singleton();

	size_t mesh_count = 0;
	size_t mesh_instance_count = 0;

	MeshRD **meshes = mesh_storage.mesh_list_get(&mesh_count);
	MeshInstanceRD **mesh_instances = mesh_storage.mesh_instance_list_get(&mesh_instance_count);

	RD &rd = RD::singleton();
	rd.draw_begin();

	for (size_t i = 0; i < mesh_instance_count; i++) {
		const MeshInstanceRD *mesh_instance = mesh_instances[i];
		const MeshRD *mesh = meshes[mesh_instance->mesh];
		for (size_t j = 0; j < mesh->primitive_count; j++) {
			const PrimitiveRD &primitive = mesh->primitives[j];
			rd.cmd_bind_index_buffer(primitive.index_buffer);
			rd.cmd_bind_vertex_buffer(primitive.vertex_buffer);
			rd.cmd_draw_indexed(primitive.index_count, 1, 0, 0, 0);
		}
	}

	rd.draw_submit();
}
