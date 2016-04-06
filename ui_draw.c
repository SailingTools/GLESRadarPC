//
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com
//

// Hello_Triangle.c
//
//    This is a simple example that draws a single triangle with
//    a minimal vertex/fragment shader.  The purpose of this 
//    example is to demonstrate the basic concepts of 
//    OpenGL ES 2.0 rendering.
#include <stdio.h>
#include <stdlib.h>
#include "esUtil.h"
#include <math.h>
#include <time.h>

#include "ui_draw.h"

#define PI (3.14159265f)

//GLfloat *blob_verts = malloc ( sizeof(GLfloat) * 3 * nVerts );
GLfloat *blob_verts;
GLfloat *line_verts;
GLfloat *circ_verts;
int nLines;
time_t start;

typedef struct
{
   // Handle to a program object
   GLuint programObject;
   GLint colorLoc; 

} UserData;

///
// Create a shader object, load the shader source, and
// compile the shader.
//
GLuint LoadShader ( GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;
   
   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
   	return 0;

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );
   
   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled ) 
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = malloc (sizeof(char) * infoLen );

         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         esLogMessage ( "Error compiling shader:\n%s\n", infoLog );            
         
         free ( infoLog );
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;

}

///
// Initialize the shader and program object
//
int Init ( ESContext *esContext )
{
   esContext->userData = malloc(sizeof(UserData));

   UserData *userData = esContext->userData;
   GLbyte vShaderStr[] =  
      "attribute vec4 vPosition;    \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = vPosition;  \n"
      "}                            \n";

   GLbyte fShaderStr[] =  
      "precision mediump float;\n"
      "uniform vec4 color;\n"
      "void main()            \n"
      "{                      \n"
      "  gl_FragColor = color;\n"
      "}                      \n";

   GLuint vertexShader;
   GLuint fragmentShader;
   GLuint programObject;
   GLint linked;

   // Load the vertex/fragment shaders
   vertexShader = LoadShader ( GL_VERTEX_SHADER, vShaderStr );
   fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, fShaderStr );

   // Create the program object
   programObject = glCreateProgram ( );
   
   if ( programObject == 0 )
      return 0;

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );

   // Bind vPosition to attribute 0   
   glBindAttribLocation ( programObject, 0, "vPosition" );

   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked ) 
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = malloc (sizeof(char) * infoLen );

         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         esLogMessage ( "Error linking program:\n%s\n", infoLog );            
         
         free ( infoLog );
      }

      glDeleteProgram ( programObject );
      return GL_FALSE;
   }

   // Store the program object
   userData->programObject = programObject;
   userData->colorLoc = glGetUniformLocation ( userData->programObject, "color" );

   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   return GL_TRUE;
}

GLfloat *blobVerts(float r, float dri, float dro, float arc_degrees, float ca, float sa)
{
    // Get the center coordinates of the inner and outer arcs
    float xm1 = (r + dri) * sa;
    float ym1 = (r + dri) * ca;
    float xm2 = (r + dro) * sa;
    float ym2 = (r + dro) * ca;

    // Get the half-widths of the inner and outer arcs
    float wi =  ((r + dri) * PI * arc_degrees / 360);
    float wo =  ((r + dro) * PI * arc_degrees / 360);

     int nVerts = 4;
     GLfloat *verts = malloc ( sizeof(GLfloat) * 3 * nVerts );
     verts[0] = xm1 + wi * ca;
     verts[1] = ym1 - wi * sa;
     verts[2] = 0.0f;
     verts[3] = xm2 + wo * ca;
     verts[4] = ym2 - wo * sa;
     verts[5] = 0.0f;
     verts[6] = xm1 - wi * ca;
     verts[7] = ym1 + wi * sa;
     verts[8] = 0.0f;
     verts[9] = xm2 - wo * ca;
     verts[10] = ym2 + wo * sa;
     verts[11] = 0.0f;

     return verts;
}

void makeBlobStencil(){
    // Generate a stencil of the vertex coordinates for all blob 
    int a, r, bi, bv, i;
    float ca, sa, angle;

    for (a=0; a<512; a++) {
        angle = ((float)a/512)*2.0*PI;
        ca = cos(angle);
        sa = sin(angle);
        //printf("a: %i, Angle: %f\n", a, angle);
        for (r=0; r<256; r++){
            bi = (256*a) + r;      // The current blob number
            bv = bi*4*3;         // The starting vertex for the current blob
            //printf("r: %i, blobIndex: %i, blobStartVert: %i\n", r, bi, bv);
            GLfloat *bverts = blobVerts(((float)r/256.0), 0.0, (1.0/256.0), (360.0/512.0), ca, sa);
            for (i=0; i<12; i++)
            {
                blob_verts[bv+i] = bverts[i];
            };
        };
    };
}

/* Function to draw a blob.  r is the mean radius and dri and dro are the inner 
 * and outer radii which are calculated as ri = r + dri.  ca and sa are the cosine
 * and sine of the mean angle of the blob respectively */
void DrawBlob(int blobIndex)
{
    // 4 vertices per blob so define the starting vertex as blobIndex*4
    glDrawArrays( GL_TRIANGLES, blobIndex*4, 3 );
    glDrawArrays( GL_TRIANGLES, blobIndex*4+1, 3 );
}


GLfloat *lineVerts(float angle, float ro, float ri){
    int nVerts = 2;
    GLfloat *verts = malloc ( sizeof(GLfloat) * 3 * nVerts );

    verts[0] = ro*sin(2.0*PI*angle/360.0);
    verts[1] = ro*cos(2.0*PI*angle/360.0);
    verts[2] = 0.0f;

    verts[3] = ri*sin(2.0*PI*angle/360.0);
    verts[4] = ri*cos(2.0*PI*angle/360.0);
    verts[5] = 0.0f;

    return verts;
}

void makeLineStencil(){

    int i, j;
    int counter = 0;
    float ri;

    // Lines for the crosshairs
    for (i=0; i<4; i++) { 
        GLfloat *verts = lineVerts(360.0*(float)i/4.0, 1.0, 0.0);
        for (j=0; j<6; j++) {
            line_verts[(counter*6)+j] = verts[j];
        };
        counter += 1;
    };

    /* Lines for the ticks */
    for (i=0; i<360; i++) 
    {
        if ( fmod(i, 10) == 0 ) { ri = 0.95; } 
        else if ( fmod(i, 5) == 0 ) { ri = 0.97; } 
        else { ri = 0.98; };
        
        GLfloat *verts = lineVerts(i, 1.0, ri);
        for (j=0; j<6; j++){
            line_verts[(counter*6)+j] = verts[j];
        };
        counter += 1;
    };

}

void DrawLines(){
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, line_verts);
    glDrawArrays( GL_LINES, 0, nLines*2 );
};

void DrawTick(float angle, float ro, float ri)
{
    int nVerts = 2;
    GLfloat *verts = malloc ( sizeof(GLfloat) * 3 * nVerts );

    verts[0] = ro*sin(2.0*PI*angle/360.0);
    verts[1] = ro*cos(2.0*PI*angle/360.0);
    verts[2] = 0.0f;

    verts[3] = ri*sin(2.0*PI*angle/360.0);
    verts[4] = ri*cos(2.0*PI*angle/360.0);
    verts[5] = 0.0f;

    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glDrawArrays( GL_LINES, 0, nVerts );
}

void makeCircStencil() {
    float radius=1.0;
    int i=0;
    int r=0;
    int vi=0;
    int nVerts = 100;

    for (r=1; r<4; r++){
        radius = ((float)r)/3.0;
        for (i=0; i<nVerts; i++)
        {
            vi = (r-1)*nVerts + i;
            circ_verts[vi*3+0] = radius*sin(2.0*PI*(float)i/(float)nVerts);
            circ_verts[vi*3+1] = radius*cos(2.0*PI*(float)i/(float)nVerts);
            circ_verts[vi*3+2] = 0.0f;
        };
    };
};

void DrawCircle(float radius)
{
    int nVerts = 100;
    int r = 0;
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, circ_verts);
    for (r=0; r<3;r++){
        glDrawArrays( GL_LINE_LOOP, r*nVerts, nVerts );
    };
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw ( ESContext *esContext )
{
    int i=0;
    float ro=1.0;
    float ri=0.95;
    int width;
    int height;
    int vpsize;
    int preserve_swap;

    eglQuerySurface(esContext->eglDisplay, esContext->eglSurface, EGL_WIDTH, &width);
    eglQuerySurface(esContext->eglDisplay, esContext->eglSurface, EGL_HEIGHT, &height);
    vpsize = width;
    if (width > height)
        vpsize = height;

   UserData *userData = esContext->userData;
      
   // Set the viewport
   glViewport ( (width/2)-(vpsize/2), (height/2)-(vpsize/2), vpsize, vpsize );

   // Preserve the already-drawn blobs
   eglQuerySurface(esContext->eglDisplay, esContext->eglSurface, EGL_SWAP_BEHAVIOR, &preserve_swap);
   if ( !(preserve_swap == EGL_BUFFER_PRESERVED) ) {
        printf("Setting EGL_SWAP_BEHAVIOR\n"); 
        eglSurfaceAttrib ( esContext->eglDisplay, esContext->eglSurface, EGL_SWAP_BEHAVIOR,  EGL_BUFFER_PRESERVED );
   };

   // Clear the color buffer
   //glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( userData->programObject );

    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, blob_verts);
    glEnableVertexAttribArray ( 0 );

    /* Draw just the current angle */
    int r, bindex, gindex;
    float bcolor;
    for (r=0; r<256; r++)
    {
        bindex = current_angle*256 + r;  // Blob index of current angle/blob
        gindex = current_angle*1024 + r;  // Blob index in global_scan_buffer
        bcolor = (float)global_scan_buffer[gindex]/120.0;
        glUniform4f ( userData->colorLoc, bcolor, 0.0, 0.0, 1.0 );
        DrawBlob(bindex);
    };
    // Increment the angle to draw next frame
    if (current_angle < 511) {
        current_angle += 1;
    } else {
        current_angle = 0;
    };
    

    /* Draw full scan buffer 
    float angle;
    float ca;
    float sa;
    float bcolor;
    int r, bindex, gindex;
    glUniform4f ( userData->colorLoc, 1.0, 0.0, 0.0, 1.0 );
    for (i=0; i<512; i++) {
        //printf("Drawing scanline for angle %i \n", i);
        for (r=0; r<256; r++){
            gindex = i*1024 + r;
            if (global_scan_buffer[gindex] > 15) {
                //printf("Drawing blob for radii %i, color: %f \n", r, bcolor);
                bindex = 256*i + r;
                //bcolor = (float)global_scan_buffer[gindex]/255.0;
                //glUniform4f ( userData->colorLoc, bcolor, 0.0, 0.0, 1.0 );
                DrawBlob(bindex);
            };
        };
        //usleep(100000);
    };*/

    /* Draw a test blobs
    for (i=0; i<512; i++){
        glUniform4f ( userData->colorLoc, 1.0, 0.0, 0.0, 1.0 );
        DrawBlob(256*i + 125);
    }

    for (i=0; i<256; i++){
        glUniform4f ( userData->colorLoc, 1.0, 0.0, 0.0, 1.0 );
        DrawBlob(256*10 + i);
    }
    glUniform4f ( userData->colorLoc, 1.0, 0.0, 0.0, 1.0 );
    float dt = time(NULL) - start;
    i = (int)dt;
    printf("rendering index: %i\n", i);
    DrawBlob(i);
    */


    /* Draw range circles and ticks */
    glUniform4f ( userData->colorLoc, 0.0, 1.0, 0.0, 1.0 );

    // Draw range circles
    //DrawCircle(1.0);
    DrawCircle(0.33333);
    //DrawCircle(0.66666);

    /* Draw the crosshairs
    for (i=0; i<4; i++) { DrawTick(360.0*(float)i/4.0, 1.0, 0.0); };

    // Draw the ticks
    for (i=0; i<360; i++) 
    {
        if ( fmod(i, 10) == 0 ) { DrawTick(i, 1.0, 0.95); }
        else if ( fmod(i, 5) == 0 ) { DrawTick(i, 1.0, 0.97); }
        else { DrawTick(i, 1.0, 0.98); };
    };*/
    DrawLines();

   // Ping the radar
   pingRadar();

}


void *drawWindow(void *arg)
{
    int i, s = 0;
    start = time(NULL);
    current_angle = 0;

    // Create the commonly used stencils
    int n_blobs = 256*512;
    int n_blob_verts = 4*3*n_blobs;  // 4 vertices per blob, 3 coordinates per vertex
    nLines = 360+4;
    blob_verts = malloc ( sizeof(GLfloat) * 3 * n_blob_verts ); 
    line_verts = malloc ( sizeof(GLfloat) * 6 * nLines );
    circ_verts = malloc ( sizeof(GLfloat) * 3 * 300);
    makeBlobStencil();
    makeLineStencil();
    makeCircStencil();

    /* Print out some vertex coordinates
    for ( i=0; i<256; i++ )
    {
        printf("Vert %i: (", i);
        for ( s=0; s<12; s++) 
        {
            printf( " %f,", blob_verts[i*12 + s] );
        };
        printf(")\n");
    };*/


   ESContext esContext;
   UserData  userData;

   esInitContext ( &esContext );
   esContext.userData = &userData;

   esCreateWindow ( &esContext, "Koden PCRadar", 720, 720, ES_WINDOW_RGB );

   if ( !Init ( &esContext ) )
      return 0;

   esRegisterDrawFunc ( &esContext, Draw );

   esMainLoop ( &esContext );
}

