#include <stdlib.h>

#include "gp.h"

/* User program */
int main()
{
    gpColor color = {255, 255, 0};

    //Create a y steep line
    gpVertex2Fixed v1 = {0, 0};
    gpVertex2Fixed v2 = {10, 600};
    gpLine(&v1, &v2, &color);

    //Create a x steep line
    v1 = (gpVertex2Fixed){0, 0};
    v2 = (gpVertex2Fixed){600, 60};
    gpLine(&v1, &v2, &color);

    //Create a y steep line in reverse
    v1 = (gpVertex2Fixed){0, 0};
    v2 = (gpVertex2Fixed){10, 600};
    gpLine(&v2, &v1, &color);

    //Create a x steep line in reverse
    v1 = (gpVertex2Fixed){0, 0};
    v2 = (gpVertex2Fixed){600, 60};
    gpLine(&v2, &v1, &color);

    //Create a -y steep line
    v1 = (gpVertex2Fixed){0, 600};
    v2 = (gpVertex2Fixed){10, 0};
    gpLine(&v1, &v2, &color);

    //Create a -x steep line
    v1 = (gpVertex2Fixed){0, 60};
    v2 = (gpVertex2Fixed){600, 0};
    gpLine(&v1, &v2, &color);

    //Create a -y steep line in reverse
    v1 = (gpVertex2Fixed){0, 600};
    v2 = (gpVertex2Fixed){10, 0};
    gpLine(&v2, &v1, &color);

    //Create a -x steep line in reverse
    v1 = (gpVertex2Fixed){0, 60};
    v2 = (gpVertex2Fixed){600, 0};
    gpLine(&v2, &v1, &color);

    // vertical line
    v1 = (gpVertex2Fixed){GP_XRES/2, 0};
    v2 = (gpVertex2Fixed){GP_XRES/2, GP_YRES-1};
    gpLine(&v2, &v1, &color);

    // horizontal line
    v1 = (gpVertex2Fixed){0, GP_YRES/2};
    v2 = (gpVertex2Fixed){GP_XRES-1, GP_YRES/2};
    gpLine(&v2, &v1, &color);
}
