#include "../common/types.h"
#define SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
#include "../third_party/spirv_reflect.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//#define SPVC_CHECKED_CALL(x) do { \
//    if ((x) != SPVC_SUCCESS) { \
//        fprintf(stderr, "Failed at line %d.\n", __LINE__); \
//        exit(1); \
//    } \
//} while(0)
//
//static u32 s_variable_count;

//static void
//dump_resource_list(spvc_compiler compiler, spvc_resources resources, spvc_resource_type type, const char* tag)
//{
//	const spvc_reflected_resource* list = NULL;
//	size_t count = 0;
//	size_t i;
//	SPVC_CHECKED_CALL(spvc_resources_get_resource_list_for_type(resources, type, &list, &count));
//	if (count == 0)
//		return;
//	s_variable_count += count;
//	printf("%s\n", tag);
//	for (i = 0; i < count; i++)
//	{
//		printf("ID: %u, BaseTypeID: %u, TypeID: %u, Name: %s\n", list[i].id, list[i].base_type_id, list[i].type_id,
//				list[i].name);
//		printf("  Set: %u, Binding: %u\n",
//				spvc_compiler_get_decoration(compiler, list[i].id, SpvDecorationDescriptorSet),
//				spvc_compiler_get_decoration(compiler, list[i].id, SpvDecorationBinding));
//	}
//}

int main(int argc, char* argv[])
{
	if (argc != 4)
		return 0;

	char* spirv_path = argv[1];
	FILE* spirv_file = fopen(spirv_path, "rb");
	assert(spirv_file);

	char* c_path = argv[3];
	FILE* c_file = fopen(c_path, "w+");
	assert(c_file);

	// Read spirv file
	fseek(spirv_file, 0, SEEK_END);
	u32 spirv_size = ftell(spirv_file);
	rewind(spirv_file);

	u8* spirv_bytes = malloc(spirv_size);
	assert(spirv_bytes);
	fread(spirv_bytes, spirv_size, 1, spirv_file);
	fclose(spirv_file);



//	spvc_context context;
//	SPVC_CHECKED_CALL(spvc_context_create(&context));
//
//	spvc_parsed_ir parsed_ir;
//	SPVC_CHECKED_CALL(spvc_context_parse_spirv(context, (u32*)spirv_bytes, spirv_size / 4, &parsed_ir));
//
//	spvc_compiler compiler;
//	SPVC_CHECKED_CALL(
//			spvc_context_create_compiler(context, SPVC_BACKEND_NONE, parsed_ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP,
//					&compiler));
//
//	spvc_resources resources;
//	SPVC_CHECKED_CALL(spvc_compiler_create_shader_resources(compiler, &resources));
//
//	spvc_set set;
//	SPVC_CHECKED_CALL(spvc_compiler_get_active_interface_variables(compiler, &set));
//
//	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_STAGE_INPUT, "===== Stage inputs =====");
//	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_STAGE_OUTPUT, "===== Stage outputs =====");
//	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, "===== Uniform buffers =====");
//	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_STORAGE_BUFFER, "===== Storage buffers =====");
//	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_SEPARATE_SAMPLERS, "===== Samplers =====");
//	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_SEPARATE_IMAGE, "===== Images =====");
//	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_STORAGE_IMAGE, "===== Storage images =====");
//	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_SAMPLED_IMAGE, "===== Sampled images =====");
//	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_PUSH_CONSTANT, "===== Push constants =====");
//	dump_resource_list(compiler, resources, SPVC_RESOURCE_TYPE_SUBPASS_INPUT, "===== Subpass inputs =====");
//
//	spvc_context_destroy(context);

	// Write byte array
	for (char* c = argv[2]; *c != 0; ++c)
	{
		if (*c == '.') *c = '_';
	}
	fprintf(c_file, "unsigned char %s[] = {\n", argv[2]);
	for (u32 i = 0; i < spirv_size; i++)
	{
		fprintf(c_file, "0x%.2X, ", spirv_bytes[i]);
		if (i % 10 == 0) fprintf(c_file, "\n");
	}
	free(spirv_bytes);
	fprintf(c_file, "};\n");

	// Write size
	fprintf(c_file, "int %s_size = %d;\n", argv[2], spirv_size);
	fclose(c_file);
	return 0;
}
