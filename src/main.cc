//==================================================================================================
// Written in 2016 by Peter Shirley <ptrshrl@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related and
// neighboring rights to this software to the public domain worldwide. This software is distributed
// without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public Domain Dedication along
// with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//==================================================================================================
#ifdef _MSC_VER
#include "msc.h"
#endif
#include <iostream>
#include "sphere.h"
#include "moving_sphere.h"
#include "hitable_list.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"
#include "box.h"
#include "surface_texture.h"
#include "aarect.h"
#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "pdf.h"
#include <iostream>
#include <fstream>

inline vec3 de_nan(const vec3& c) {
    vec3 temp = c;
    if (!(temp[0] == temp[0])) temp[0] = 0;
    if (!(temp[1] == temp[1])) temp[1] = 0;
    if (!(temp[2] == temp[2])) temp[2] = 0;
    return temp;
}



vec3 color(const ray& r, hitable *world, hitable *light_shape, int depth) {
    hit_record hrec;
    if (world->hit(r, 0.001, MAXFLOAT, hrec)) {
        scatter_record srec;
        vec3 emitted = hrec.mat_ptr->emitted(r, hrec, hrec.u, hrec.v, hrec.p);
        if (depth < 50 && hrec.mat_ptr->scatter(r, hrec, srec)) {
            if (srec.is_specular) {
                return srec.attenuation * color(srec.specular_ray, world, light_shape, depth+1);
            }
            else {
                hitable_pdf plight(light_shape, hrec.p);
                mixture_pdf p(&plight, srec.pdf_ptr);
                ray scattered = ray(hrec.p, p.generate(), r.time());
                float pdf_val = p.value(scattered.direction());
                delete srec.pdf_ptr;
                return emitted + srec.attenuation*hrec.mat_ptr->scattering_pdf(r, hrec, scattered)*color(scattered, world, light_shape, depth+1) / pdf_val;
            }
        }
        else
            return emitted;
    }
    else
        return vec3(0,0,0);
}

void cornell_box(hitable **scene, camera **cam, float aspect) {
    int i = 0;
    hitable **list = new hitable*[8];
    material *red = new lambertian( new constant_texture(vec3(0.65, 0.05, 0.05)) );
    material *white = new lambertian( new constant_texture(vec3(0.73, 0.73, 0.73)) );
    material *green = new lambertian( new constant_texture(vec3(0.12, 0.45, 0.15)) );
    material *light = new diffuse_light( new constant_texture(vec3(15, 15, 15)) );
    list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
    list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
    list[i++] = new flip_normals(new xz_rect(213, 343, 227, 332, 554, light));
    list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
    list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
    list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));
    material *glass = new dielectric(1.5);
    list[i++] = new sphere(vec3(190, 90, 190),90 , glass);
    list[i++] = new translate(new rotate_y(
                    new box(vec3(0, 0, 0), vec3(165, 330, 165), white),  15), vec3(265,0,295));
    *scene = new hitable_list(list,i);
    vec3 lookfrom(278, 278, -800);
    vec3 lookat(278,278,0);
    float dist_to_focus = 10.0;
    float aperture = 0.0;
    float vfov = 40.0;
    *cam = new camera(lookfrom, lookat, vec3(0,1,0),
                      vfov, aspect, aperture, dist_to_focus, 0.0, 1.0);
}

int main() {
    int startTime = clock();
    std::ofstream out;
    out.open("OutputPic.ppm");


    int nx = 500;
    int ny = 500;
    int ns = 10;
    out << "P3\n" << nx << " " << ny << "\n255\n";
    hitable *world;
    camera *cam;
    float aspect = float(ny) / float(nx);
    cornell_box(&world, &cam, aspect);
    hitable *light_shape = new xz_rect(213, 343, 227, 332, 554, 0);
    hitable *glass_sphere = new sphere(vec3(190, 90, 190), 90, 0);
    hitable *a[2];
    a[0] = light_shape;
    a[1] = glass_sphere;
    hitable_list hlist(a,2);

    for (int j = ny-1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            vec3 col(0, 0, 0);
            for (int s=0; s < ns; s++) {
                float u = float(i+drand48())/ float(nx);
                float v = float(j+drand48())/ float(ny);
                ray r = cam->get_ray(u, v);
                vec3 p = r.point_at_parameter(2.0);
                col += de_nan(color(r, world, &hlist, 0));
            }
            col /= float(ns);
            col = vec3( sqrt(col[0]), sqrt(col[1]), sqrt(col[2]) );
            int ir = int(255.99*col[0]);
            int ig = int(255.99*col[1]);
            int ib = int(255.99*col[2]);
            out << ir << " " << ig << " " << ib << "\n";
        }
        float costTime = (float)(clock() - startTime)/ (float) CLOCKS_PER_SEC / 60.0f;
        float nowProgress = ((ny - j) * 1.0f) / (ny * 1.0f);
        std::cout<<"Progress:"<< nowProgress * 100 << "% "
                 << "Already cost:" << costTime << " minutes"
                 << " Still need:" << costTime / nowProgress * (1-nowProgress)<< " minutes\n";
    }
    std::ofstream outTime;
    outTime.open("OutputTime.txt");

    outTime << "Width:" << nx << " Height:" << ny << " Samples:" << ns << "\n";
    outTime << float(clock() - startTime) / (float) CLOCKS_PER_SEC / 60.0f << " minutes\n";
    out.close();


    // 打开结果图片
#ifdef _WIN32
    system(" start .\\OutputPic.ppm");
    //define something for Windows (32-bit and 64-bit, this part is common)
#ifdef _WIN64
    //define something for Windows (64-bit only)
#else
    //define something for Windows (32-bit only)
#endif
#elif __APPLE__

    #include "TargetConditionals.h"

#if TARGET_IPHONE_SIMULATOR
    // iOS Simulator
#elif TARGET_OS_IPHONE
    // iOS device
#elif TARGET_OS_MAC
    // Other kinds of Mac OS
    FILE *pp = popen("open OutputPic.ppm", "r");
#else
#   error "Unknown Apple platform"
#endif
#elif __ANDROID__
    // android
#elif __linux__
    // linux
#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif
}

