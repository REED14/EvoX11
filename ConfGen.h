#include <X11/extensions/render.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/shape.h>

//variables and drawing function

int time_conf;
int is_conf = 0;
int conf_depth_x;
int conf_depth_y;
int distort_x = 0;
int distort_y = 0;
Window ConfigWin;

int GlobalRadius = 20;

//i just need this
static Picture
solid_white_picture (Display *dpy, Window root)
{
    Pixmap			pixmap;
    Picture			picture;
    XRenderPictureAttributes	pa;
    XRenderColor		c;

    pixmap = XCreatePixmap (dpy, root, 1, 1, 32);
    if (!pixmap)
	return None;

    pa.repeat = True;
    picture = XRenderCreatePicture (dpy, pixmap,
				    XRenderFindStandardFormat (dpy, PictStandardARGB32),
				    CPRepeat,
				    &pa);
    if (!picture)
    {
	XFreePixmap (dpy, pixmap);
	return None;
    }

    c.alpha = 0xffff;
    c.red = 0xffff;
    c.green = 0xffff;
    c.blue = 0xffff;
    XRenderFillRectangle (dpy, PictOpSrc, picture, &c, 0, 0, 1, 1);
    XFreePixmap (dpy, pixmap);
    return picture;
}

//mask rounded corners picture
Picture RoundedCorners(Display* dpy, int wid, int hei, Window root, Picture alpha, int radius){
    Pixmap			pixmap;
    Picture			picture;
    XRenderColor		c, c1;

    pixmap = XCreatePixmap (dpy, root, wid, hei, 32);
    if (!pixmap)
	    return None;

    picture = XRenderCreatePicture (dpy, pixmap,
				    XRenderFindStandardFormat (dpy, PictStandardARGB32),
				    0,
				    NULL);

    if (!picture)
    {
        XFreePixmap (dpy, pixmap);
        return None;
    }

    c.alpha = 0 * 0xffff;
    c.red = 0 * 0xffff;
    c.green = 0 * 0xffff;
    c.blue = 0 * 0xffff;

    
    c1.alpha = 1 * 0xffff;
    c1.red = 1 * 0xffff;
    c1.green = 1 * 0xffff;
    c1.blue = 1 * 0xffff;

    
    XRenderComposite(dpy, PictOpSrc, alpha, None, picture, 0, 0, 0, 0,
                    0, 0, wid, hei);

    ///left up
    for(int posy = 0; posy<=radius; posy++){
        int size_wid = (1.0-sqrt(1-((radius-posy)*(radius-posy)*1.0/(radius*radius))))*radius;
        XRenderFillRectangle (dpy, PictOpSrc, picture, &c, 0, posy, size_wid, 1);

	//vertical antialiasing
	if(posy>=radius/5){
	  if(((1.0-sqrt(1-((radius-posy)*(radius-posy)*1.0/(radius*radius)))))*radius-size_wid>0.3){  
	    XRenderColor c2;

	    double AliasingFactor = 1-((1.0-sqrt(1-((radius-posy)*(radius-posy)*1.0/(radius*radius))))*radius-size_wid);

	    c2.alpha = AliasingFactor * 0xffff;
	    c2.red = AliasingFactor * 0xffff;
	    c2.green = AliasingFactor * 0xffff;
	    c2.blue = AliasingFactor * 0xffff;
	  
	    XRenderFillRectangle (dpy, PictOpSrc, picture, &c2, size_wid, posy, 1, 1);
	  }
	}
	//horizontal antialiasing
	else {
	  int size_wid_next = (1.0-sqrt(1-((radius-posy-1)*(radius-posy-1)*1.0/(radius*radius))))*radius;
	  int swd = size_wid-size_wid_next;

	  if(swd!=0)
	  for(int i=size_wid_next; i<=size_wid; i++){
	    double AliasingFactor;
	    AliasingFactor = ((i-size_wid_next-1)*1.0/swd+(i-size_wid_next)*1.0/swd)/2;
	      
	    XRenderColor c2;
	  
	    c2.alpha = AliasingFactor * 0xffff;
	    c2.red = AliasingFactor * 0xffff;
	    c2.green = AliasingFactor * 0xffff;
	    c2.blue = AliasingFactor * 0xffff;

	    if(AliasingFactor>0)

	    XRenderFillRectangle (dpy, PictOpSrc, picture, &c2, i, posy, 1, 1);
	  }
	}
    }

    ///right up
    for(int posy = 0; posy<=radius; posy++){
        int size_wid = (1.0-sqrt(1-((radius-posy)*(radius-posy)*1.0/(radius*radius))))*radius;

        XRenderFillRectangle (dpy, PictOpSrc, picture, &c, wid-size_wid, posy, size_wid, 1);
	
	//vertical antialiasing
	if(posy>=radius/5){
	  if(((1.0-sqrt(1-((radius-posy)*(radius-posy)*1.0/(radius*radius)))))*radius-size_wid>0.3){  
	    XRenderColor c2;

	    double AliasingFactor = 1-((1.0-sqrt(1-((radius-posy)*(radius-posy)*1.0/(radius*radius))))*radius-size_wid);

	    c2.alpha = AliasingFactor * 0xffff;
	    c2.red = AliasingFactor * 0xffff;
	    c2.green = AliasingFactor * 0xffff;
	    c2.blue = AliasingFactor * 0xffff;

	    if(AliasingFactor>0)
	    XRenderFillRectangle (dpy, PictOpSrc, picture, &c2, wid-size_wid-1, posy, 1, 1);
	  }
	}
	//horizontal antialiasing
	else {
	  int size_wid_next = (1.0-sqrt(1-((radius-posy-1)*(radius-posy-1)*1.0/(radius*radius))))*radius;
	  int swd = size_wid-size_wid_next;

	  if(swd!=0)
	  for(int i=size_wid_next; i<=size_wid; i++){
	    double AliasingFactor;
	    AliasingFactor = ((i-size_wid_next-1)*1.0/swd+(i-size_wid_next)*1.0/swd)/2;
	      
	    XRenderColor c2;
	  
	    c2.alpha = AliasingFactor * 0xffff;
	    c2.red = AliasingFactor * 0xffff;
	    c2.green = AliasingFactor * 0xffff;
	    c2.blue = AliasingFactor * 0xffff;

	    if(AliasingFactor>0)
	    XRenderFillRectangle (dpy, PictOpSrc, picture, &c2, wid-i-1, posy, 1, 1);
	  }
	}
    }

    ///left down
    for(int posy = 0; posy<=radius; posy++){
        int size_wid = (1.0-sqrt(1-((radius-posy)*(radius-posy)*1.0/(radius*radius))))*radius;
        XRenderFillRectangle (dpy, PictOpSrc, picture, &c, 0, hei-posy-1, size_wid, 1);
		
	//vertical antialiasing
	if(posy>=radius/5){
	  if(((1.0-sqrt(1-((radius-posy)*(radius-posy)*1.0/(radius*radius)))))*radius-size_wid>0.3){  
	    XRenderColor c2;

	    double AliasingFactor = 1-((1.0-sqrt(1-((radius-posy)*(radius-posy)*1.0/(radius*radius))))*radius-size_wid);

	    //printf("%f\n", AliasingFactor);
	    c2.alpha = AliasingFactor * 0xffff;
	    c2.red = AliasingFactor * 0xffff;
	    c2.green = AliasingFactor * 0xffff;
	    c2.blue = AliasingFactor * 0xffff;

	    if(AliasingFactor>0)
	      XRenderFillRectangle (dpy, PictOpSrc, picture, &c2, size_wid, hei-posy-1, 1, 1);
	  }
	}
	
	//horizontal antialiasing
	else {
	  int size_wid_next = (1.0-sqrt(1-((radius-posy-1)*(radius-posy-1)*1.0/(radius*radius))))*radius;
	  int swd = size_wid-size_wid_next;

	  if(swd!=0)
	  for(int i=size_wid_next; i<=size_wid; i++){
	    double AliasingFactor;
	    AliasingFactor = ((i-size_wid_next-1)*1.0/swd+(i-size_wid_next)*1.0/swd)/2;
	      
	    XRenderColor c2;
	  
	    c2.alpha = AliasingFactor * 0xffff;
	    c2.red = AliasingFactor * 0xffff;
	    c2.green = AliasingFactor * 0xffff;
	    c2.blue = AliasingFactor * 0xffff;

	    if(AliasingFactor>0)
	      XRenderFillRectangle (dpy, PictOpSrc, picture, &c2, i, hei-posy-1, 1, 1);
	  }
	}
    }

    ///right down
    for(int posy = 0; posy<=radius; posy++){
        int size_wid = (1.0-sqrt(1-((radius-posy)*(radius-posy)*1.0/(radius*radius))))*radius;
        XRenderFillRectangle (dpy, PictOpSrc, picture, &c, wid-size_wid, hei-posy-1, size_wid, 1);
   		
	//vertical antialiasing
	if(posy>=radius/5){
	  if(((1.0-sqrt(1-((radius-posy)*(radius-posy)*1.0/(radius*radius)))))*radius-size_wid>0.3){  
	    XRenderColor c2;

	    double AliasingFactor = 1-((1.0-sqrt(1-((radius-posy)*(radius-posy)*1.0/(radius*radius))))*radius-size_wid);

	    //printf("%f\n", AliasingFactor);
	    c2.alpha = AliasingFactor * 0xffff;
	    c2.red = AliasingFactor * 0xffff;
	    c2.green = AliasingFactor * 0xffff;
	    c2.blue = AliasingFactor * 0xffff;

	    if(AliasingFactor>0)
	      XRenderFillRectangle (dpy, PictOpSrc, picture, &c2, wid-size_wid-1, hei-posy-1, 1, 1);
	  }
	}
	
	//horizontal antialiasing
	else {
	  int size_wid_next = (1.0-sqrt(1-((radius-posy-1)*(radius-posy-1)*1.0/(radius*radius))))*radius;
	  int swd = size_wid-size_wid_next;

	  if(swd!=0)
	  for(int i=size_wid_next; i<=size_wid; i++){
	    double AliasingFactor;
	    AliasingFactor = ((i-size_wid_next-1)*1.0/swd+(i-size_wid_next)*1.0/swd)/2;
	      
	    XRenderColor c2;
	  
	    c2.alpha = AliasingFactor * 0xffff;
	    c2.red = AliasingFactor * 0xffff;
	    c2.green = AliasingFactor * 0xffff;
	    c2.blue = AliasingFactor * 0xffff;

	    if(AliasingFactor>0)
	      XRenderFillRectangle (dpy, PictOpSrc, picture, &c2, wid-i-1, hei-posy-1, 1, 1);
	  }
	}
    }

    XFreePixmap(dpy, pixmap);
    return picture;
}

void CompConf(Display *dpy, Picture p, Picture rb, Picture maskPict, int x, int y, int wid, int hei, int now, 
              Window root, int rw, int rh)
{
	XRenderPictureAttributes	pa;
	XRenderPictFormat		*format;
    format = XRenderFindStandardFormat (dpy, PictStandardARGB32);

    Pixmap temp_px = XCreatePixmap(dpy, root, rw, rh, 32);
    Picture temp_p = XRenderCreatePicture(dpy, temp_px, 
                     format, 0, NULL);

    XRenderColor c;
    c.alpha = 0;
    c.red = 0;
    c.green = 0;
    c.blue = 0;
    XRenderFillRectangle (dpy, PictOpSrc, temp_p, &c, 0, 0, rw, rh);

    XRectangle  r;
	r.x = 0;
	r.y = 0;
	r.width = rw;
	r.height = rh;
	XserverRegion region = XFixesCreateRegion (dpy, &r, 1);
    
    if(is_conf && now-time_conf<5){
        distort_x = (distort_x*4-conf_depth_x*80)/8;
        distort_y = (distort_y*4+conf_depth_y*80)/8;
    }
    else{
        distort_x = (distort_x)/6;
        distort_y = (distort_y)/6;
    }

    if(distort_x > 300) distort_x = 300;
    if(distort_y > 100) distort_y = 100;

    if(maskPict!=None)
        maskPict = RoundedCorners(dpy, wid, hei, root, solid_white_picture(dpy, root), GlobalRadius);

    if(distort_x<0){
        XFixesSetPictureClipRegion (dpy, temp_p, 0, 0, region);

        for(int onx=0; onx<=hei; onx++){
            int xadder = (onx*onx*distort_x)/(hei*hei);

            XRenderComposite(dpy, PictOpSrc, p, maskPict, temp_p,
                            0, onx, 0, onx, xadder-distort_x, onx, wid, 1);
        }

        XFixesSetPictureClipRegion (dpy, rb, 0, 0, region);

        for(int ony=0; ony<=wid-distort_x; ony++){
            int yadder = sin(ony*3.14/(wid-distort_x))*distort_y;

            XRenderComposite(dpy, PictOpOver, temp_p, None, rb,
                            ony, 0, 0, 0, x+ony+distort_x, y-yadder, 1, hei);
        }
    }

    else{
        XFixesSetPictureClipRegion (dpy, temp_p, 0, 0, region);

        for(int onx=0; onx<=hei; onx++){
            int xadder = (onx*onx*distort_x)/(hei*hei);
            XRenderComposite(dpy, PictOpSrc, p, maskPict, temp_p,
                            0, onx, 0, onx, xadder, onx, wid, 1);
        }

        XFixesSetPictureClipRegion (dpy, rb, 0, 0, region);

        for(int ony=0; ony<=wid+distort_x; ony++){
            int yadder = sin(ony*3.14/(wid-distort_x))*distort_y;
            XRenderComposite(dpy, PictOpOver, temp_p, None, rb,
                            ony, 0, 0, 0, x+ony, y-yadder, 1, hei);
        }
    }

    XFreePixmap(dpy, temp_px);
    XRenderFreePicture(dpy, temp_p);
    XRenderFreePicture(dpy, maskPict);
    maskPict = None;
}
