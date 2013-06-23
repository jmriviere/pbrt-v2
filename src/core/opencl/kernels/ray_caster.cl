/*
 * ray_caster.cl
 *
 *  Created on: Jun 21, 2013
 *      Author: poupine
 */

typedef struct s_ray {
	float3 origin;
	float3 direction;
} Ray;

typedef struct s_sphere {
	float3 center;
	float radius;
} Sphere;

float ray_sphere_intersection(Ray ray, Sphere sphere) {
	float3 oc = ray.origin - sphere.center;
	float A = dot(ray.direction, ray.direction);
	float B = 2 * dot(d, oc);
	float C = dot(oc, oc);

	float inv2A = 1/(2A);

	float delta = B*B - 4*A*C;

	if (delta < 0) {
		return -1;
	}
	else {
		float sqrt_d = sqrt(delta);
		float t1 = (-B - sqrt_d) * inv2A;
		float t2 = (-B + sqrt_d) * inv2A;
		return (t1 > t2 ? t1 : t2);
	}
}

__kernel void ray_cast(__global image2d_t out, __read_only Ray* rays, int nb_prim, __read_only Sphere* spheres) {

	int x = get_global_id(0);
	int y = get_global_id(1);

	for (int i = 0; i < nb_prim; ++i) {
		if (ray_sphere_intersection(ray[x][y], spheres[i]) != -1) {
			write_imagef(out, (x,y), (float3) (1, 0, 0));
		}
	}

}
