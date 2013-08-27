/*
 * ray_caster.cl
 *
 *  Created on: Jun 21, 2013
 *      Author: poupine
 */

#include "ray_caster.h"

inline bool ray_sphere_intersection(Hit* hit, Ray ray, Metadata m_sphere, __global float* prims) {
  Sphere sphere;
  sphere.radius = prims[m_sphere.offset];

  Ray trans = ray;
  trans.origin = transform_point(ray.origin, m_sphere.fromWorld);
  trans.direction = transform_vect(ray.direction, m_sphere.fromWorld);

  float A = dot(trans.direction, trans.direction);
  float B = 2 * dot(trans.direction, trans.origin);
  float C = dot(trans.origin, trans.origin) - sphere.radius * sphere.radius;

  float delta = B*B - 4*A*C;

  if (delta < 0) {
    return false;
  }

  float sqrt_d = sqrt(delta);

  float q;

  if (B < 0)
    q = -0.5f * (B - sqrt_d);
  else
    q = -0.5f * (B + sqrt_d);

  float t0 = q/A;
  float t1 = C/q;

  // make sure t0 is smaller than t1
  if (t0 > t1)
    {
      // if t0 is bigger than t1 swap them around
      float temp = t0;
      t0 = t1;
      t1 = temp;
    }

  // if t1 is less than zero, the object is in the ray's negative direction
  // and consequently the ray misses the sphere
  if (t1 < 0)
    return false;

  // if t0 is less than zero, the intersection point is at t1
  if (t0 < 0)
    {
      hit->t = t1;
    }
  // else the intersection point is at t0
  else
    {
      hit->t = t0;
    }

  return true;
}

inline bool intersect(Hit* hit, Ray ray, __global Metadata* meta_prims,
						__global float* prims, int nb_prims) {
  float temp;
  hit->t = 1e5f;
  hit->selfisect = false;

  for (uint i = 0; i < nb_prims - 1; ++i) {
    switch (meta_prims[i].type) {
    case sphere:
      temp = hit->t;
      if (ray_sphere_intersection(hit, ray, meta_prims[i], prims) && hit->t < temp) {
	if (hit->id == i && hit->t < 1e-2f)
	  hit->selfisect = true;
	hit->id = i;
      }
      else {
	hit->t = temp;
      }
      break;
    default:
      break;
    }
  }
  return hit->t < 1e5f;
}


inline Color lookup(image2d_t env, Ray ray) {
  float3 dir = normalize(ray.direction);
  dir.s2 = (dir.s2 > 1.f ? 1.f : (dir.s2 < -1.f ? -1.f : dir.s2));
  float theta = acos(dir.s2);
  float phi = atan2(dir.s1, dir.s0);
  float x = (phi < 0.f ? phi + 2.f * M_PI : phi)/(2.f * M_PI);
  float y = theta/M_PI;
  Color c = read_imagef(env, sampler, (float2)(x,y));
  return c;
}

Color radiance(image2d_t env, Ray ray, __global Metadata* meta_prims, __global float* prims,
	       int nb_prims, RNG* rng, __global const Distribution1D* pConditionalV,
	       Distribution1D pMarginal, __global const float* cdfConditionalV,
	       __global const float* cdfMarginal, __global const float* fun2D,
	       __global const float* fun1D) {

  Color reflectance = (Color)(1, 1, 1, 1);
  Color cl = (Color)(0,0,0,0);

  threefry4x32_ctr_t r;

  Hit hit;

  uint i = 0;
  float3 hitpoint;

  int bounces = 2;

  while(true) {
    rng->c.v[0]++;
    rng->c.v[1]++;

    r = threefry4x32(rng->c, rng->k);

    float rand = u01_open_open_32_24(r.v[0]);

    if (!intersect(&hit, ray, meta_prims, prims, nb_prims)) {
      ray.direction = transform_vect(ray.direction, meta_prims[nb_prims - 1].fromWorld);
      cl += reflectance * lookup(env, ray);
      break;
    }

    if (i++ >= 3) {
      if (rand < 0.4) {
	reflectance /= 0.6;
      }
      else {
	break;
      }
    }

    if (hit.t < 1e-2f && hit.selfisect) {
      ray.origin += 1e-2f * ray.direction;
      continue;
    }

    hitpoint = ray.origin + hit.t * ray.direction; // hitpoint in world space
    float3 n = normalize(transform_point(hitpoint, meta_prims[hit.id].fromWorld));
    ray.origin = hitpoint;

    //		//switch (meta_prims[hit.id].mat) {
    switch (hit.id) {
      		case 0:
    default:
      			ray.direction = reflection(ray, n);
      			bounces = 2;
      			break;
      //		case REFR:
			//    case 1:
      {
	bounces = 5;
	float rand = u01_open_open_32_24(r.v[1]);
	ray.direction = refraction(&reflectance, ray, n, rand);
	break;
      }
      //    case 2:
      //default:
    diffuse:
      {
	bounces = 2;
	float u1 = u01_open_open_32_24(r.v[0]);
	float u2 = u01_open_open_32_24(r.v[1]);

	float mapPdf;

	float2 sample = sampleContinuous2D(u1, u2, pConditionalV, pMarginal, cdfConditionalV,
					   cdfMarginal, fun2D, fun1D, &mapPdf);

	if (sample.x < 0 || sample.y < 0)
	  printf((__constant char *)"%v2f\n", sample);

	sample *= sph_coord;

	float costheta = cos(sample.x), sintheta = sin(sample.x);
	float sinphi = sin(sample.y), cosphi = cos(sample.y);

	float cosi = dot(-ray.direction, n);

	float3 direction = normalize((float3)(sintheta * cosphi, sintheta * sinphi, costheta));

	if (cosi < 0 || mapPdf == 0.f || sintheta == 0.f)
	  continue;

	ray.direction = direction;

	reflectance *= 2.f * M_PI * sintheta * cosi/mapPdf;
	//				float pX = mapPdf/(2.f * M_PI * M_PI * sintheta * cosi);
	//				float w = 256 * .25 * pX / (256 * .25 * pX + 256 * .75 * 5.f/(2 * M_PI) * pow(costheta, 4) * sintheta);
	//				reflectance *= w / (0.25f * pX);
	break;
      }

      //    case 3:
      //      		default:
      {
	float criterion = u01_open_open_32_24(r.v[1]);
	float3 z = reflection(ray, n);

	float eps1 = u01_open_open_32_24(r.v[2]);
	float eps2 = u01_open_open_32_24(r.v[3]);

	float theta = acos(pow(eps1, 0.02f));
	float phi = 2.0f * M_PI * eps2;

	if (criterion < 0.75f) {
	  bounces = 2;

	  float xs = sin(theta) * cos(phi);
	  float ys = sin(theta) * sin(phi);
	  float zs = cos(theta);

	  float3 w = z;

	  float3 u = (float3)normalize(cross((float3)(1, 0, 0),w));
	  float3 v = (float3)normalize(cross(u,w));

	  ray.direction = normalize((float3)(xs * u + ys * v + zs * w));
	  float pX = 50.f/(2 * M_PI) * pow(cos(theta), 49) * sin(theta);
	  float cosi = dot(-ray.direction, n);
	  float we = 128 * 0.75 * pX / (128 * 0.75 * pX + 0.25 * 128 * probability(eps1, eps2,
										   pConditionalV, pMarginal,
										   fun2D, fun1D));
	  	  reflectance *= we/(.75f * pX);
	  //reflectance *= 50.f/(2 * M_PI) * pow(cos(theta), 49) * sin(theta);
	}
	else {
	  //					goto diffuse;
	  float mapPdf;
	float2 sample = sampleContinuous2D(eps1, eps2, pConditionalV, pMarginal, cdfConditionalV,
					   cdfMarginal, fun2D, fun1D, &mapPdf);

	if (sample.x < 0 || sample.y < 0)
	  printf((__constant char *)"%v2f\n", sample);

	sample *= sph_coord;

	float costheta = cos(sample.x), sintheta = sin(sample.x);
	float sinphi = sin(sample.y), cosphi = cos(sample.y);

	float cosi = dot(-ray.direction, n);

	float3 direction = normalize((float3)(sintheta * cosphi, sintheta * sinphi, costheta));

	if (cosi < 0 || mapPdf == 0.f || sintheta == 0.f)
	  continue;

	ray.direction = direction;

	//	reflectance *= 2.f * M_PI * sintheta * cosi/mapPdf;
	float pX = mapPdf/(2.f * M_PI * M_PI * sintheta * cosi);
					float w = 128 * 0.25f * pX / (128 * .25f * pX + 128 * 0.75f * 50.f/(2 * M_PI) * pow(costheta, 49) * sintheta);
					reflectance *= w / (0.25f * pX);
	}
	break;
      }
      //		default:
      //			break;
    }
  }
  return cl;
}

__kernel void ray_cast(__global float4* Ls, __global OCLCamera* cam, uint width, int spp,
				int nb_prims, __global Metadata* meta_prims, __global float* prims,
		       __read_only image2d_t env, __global const Distribution1D* pConditionalV,
		       __global Distribution1D* pMarginal, __global const float* cdfConditionalV,
		       __global const float* cdfMarginal, __global const float* fun2D,
		       __global const float* fun1D) {

  int xPos = get_global_id(0);
  int yPos = get_global_id(1);

  RNG rng;

  threefry4x32_key_t k = {{xPos, 0xdecafbad, 0xfacebead, 0x12345678}};
  threefry4x32_ctr_t c = {{yPos, 0xf00dcafe, 0xdeadbeef, 0xbeeff00d}};

  rng.k = k;
  rng.c = c;

  threefry4x32_ctr_t r;


  float sample_x, sample_y;
  float rand_x, rand_y;

  Color pixel = (Color)(0, 0, 0, 0);

  for (uint i = 0; i < spp; i++) {
    r = threefry4x32(rng.c , rng.k);

    rand_x = u01_open_open_32_24(r.v[0]);
    rand_y = u01_open_open_32_24(r.v[1]);

    sample_x = ((float)xPos + rand_x);
    sample_y = ((float)yPos + rand_y);

    Ray ray = generate_ray(*cam, (float2)(sample_x, sample_y));

    pixel += radiance(env, ray, meta_prims, prims, nb_prims, &rng, pConditionalV,
		      *pMarginal, cdfConditionalV, cdfMarginal, fun2D, fun1D);
  }

  Ls[yPos * width + xPos] = pixel/spp;

}
