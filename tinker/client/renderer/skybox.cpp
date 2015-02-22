#include "skybox.h"

int asset_skybox_vert_stride_bytes = 5 * sizeof(float);
int asset_skybox_num_verts = 36;
int asset_skybox_uv_offset_bytes = 3 * sizeof(float);

float asset_skybox_verts[] = {
	100, 100, -100, 0, 0,
	100, -100, -100, 1, 0,
	100, -100, 100, 1, 1,
	100, 100, 100, 0, 1,

	-100, -100, -100, 0, 0,
	-100, 100, -100, 1, 0,
	-100, 100, 100, 1, 1,
	-100, -100, 100, 0, 1,

	-100, 100, -100, 0, 0,
	100, 100, -100, 1, 0,
	100, 100, 100, 1, 1,
	-100, 100, 100, 0, 1,

	100, -100, -100, 0, 0,
	-100, -100, -100, 1, 0,
	-100, -100, 100, 1, 1,
	100, -100, 100, 0, 1,

	100, 100, 100, 0, 0,
	100, -100, 100, 1, 0,
	-100, -100, 100, 1, 1,
	-100, 100, 100, 0, 1,

	-100, 100, -100, 0, 0,
	-100, -100, -100, 1, 0,
	100, -100, -100, 1, 1,
	100, 100, -100, 0, 1,
};

unsigned int asset_skybox_indxs[] = {
	0, 1, 2,
	0, 2, 3,

	4, 5, 6,
	4, 6, 7,

	8, 9, 10,
	8, 10, 11,

	12, 13, 14,
	12, 14, 15,

	16, 17, 18,
	16, 18, 19,

	20, 21, 22,
	20, 22, 23,
};

int asset_skybox_verts_size_bytes = sizeof(asset_skybox_verts);
int asset_skybox_indxs_size_bytes = sizeof(asset_skybox_indxs);

