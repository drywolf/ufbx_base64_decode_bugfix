#include "ufbx.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include <assert.h>
#include <string.h>

#include "ufbx.c"       // (base64 BUGGED) <--- original ufbx base64 decoding
// #include "ufbx_fixed.c" // (base64 FIXED)  <--- fixed base64 decoding (see Assimp FBX import code)

const char* get_filename(ufbx_string path);

int main(int argc, char** argv)
{
	ufbx_load_opts opts = { 0 };
	ufbx_error error;
	ufbx_scene* scene = ufbx_load_file("GoonActorCore.fbx", &opts, &error);
	if (!scene) {
		fprintf(stderr, "Failed to load: %s\n", error.description.data);
		return 1;
	}

	ufbx_material* material = scene->materials.data[0];
	material->textures.count;

	int encountered_errors = 0;

	for (int i = 0; i < material->textures.count; i++) {
		const ufbx_material_texture material_texture = material->textures.data[i];
		const ufbx_texture* texture = material_texture.texture;

		const char* texture_filename = get_filename(texture->filename);

		FILE* save_texture_file = fopen(texture_filename, "wb");
		size_t written_bytes = fwrite(texture->content.data, sizeof(char), texture->content.size, save_texture_file);
		assert(written_bytes == texture->content.size);
		fclose(save_texture_file);

		int width, height, channels;
		stbi_uc* pixels = stbi_load_from_memory(texture->content.data, texture->content.size, &width, &height, &channels, STBI_rgb_alpha);

		if (pixels) {
			printf("STBI SUCCESS decoding texture '%s'\n", texture_filename);
		}
		else {
			const char* error_reason = stbi_failure_reason();
			printf("STBI ERROR decoding texture '%s'   (reason: %s)\n", texture_filename, error_reason);
			++encountered_errors;
		}

		stbi_image_free(pixels);
	}

	return encountered_errors;
}

const char* get_filename(ufbx_string path) {
	for (int i = path.length - 1; i > 0; --i) {
		if (path.data[i] == '/' || path.data[i] == '\\') {
			return &path.data[i + 1];
		}
	}

	return path.data;
}
