# EvoX11
This is a fork of XCompmgr with wind effects on moving windows (similar to wobbly windows in Compiz), expanding on window mapping and shrinkage on window unmapping, rounded corneners and custom background for transparent windows(png format).

# Dependencies
The same dependencies of XCompmgr + Cairo

# Installation process

```bash
make
make install
```

# Usage
```
-d display
      Specifies which display should be managed.
-r radius
      Specifies the blur radius for client-side shadows. (default 12)
 -o opacity
      Specifies the translucency for client-side shadows. (default .75)
-l left-offset
      Specifies the left offset for client-side shadows. (default -15)
-t top-offset
      Specifies the top offset for clinet-side shadows. (default -15)
-I fade-in-step
      Specifies the opacity change between steps while fading in. (default 0.028)
-O fade-out-step
      Specifies the opacity change between steps while fading out. (default 0.03)
-D fade-delta-time
      Specifies the time between steps in a fade in milliseconds. (default 10)
-a
      Use automatic server-side compositing. Faster, but no special effects.
-c
      Draw client-side shadows with fuzzy edges.
-C
      Avoid drawing shadows on dock/panel windows.
-f
      Fade windows in/out when opening/closing.
-F
      Fade windows during opacity changes.
-n
      Normal client-side compositing with transparency support.
-s
      Draw server-side shadows with sharp edges.
-S
      Enable synchronous operation (for debugging).
-B path-to-background-png
      Specify the absolute path of background png for transparent windows.
-R rounded-corner-radius
      Specify the radius of the rounded corners of the windows. (default 20)
```
