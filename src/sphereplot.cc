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

#include <iostream>
#include <math.h>
#include "msc.h"

int main0004() {
    for (int i = 0; i < 2000; i++) {
        float r1 = drand48();
        float r2 = drand48();
        float x = cos(2*M_PI*r1)*2*sqrt(r2*(1-r2));
        float y = sin(2*M_PI*r1)*2*sqrt(r2*(1-r2));
        float z = 1 - 2*r2;
        std::cout << x << " " << y << " " << z << "\n";
    }
    return 0;
}

