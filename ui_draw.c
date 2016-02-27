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
#include <stdlib.h>
#include "esUtil.h"
#include <math.h>

#define PI (3.14159265f)

typedef struct
{
   // Handle to a program object
   GLuint programObject;

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
      "precision mediump float;\n"\
      "void main()                                  \n"
      "{                                            \n"
      "  gl_FragColor = vec4 ( 0.0, 1.0, 0.0, 1.0 );\n"
      "}                                            \n";

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

   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   return GL_TRUE;
}

/* Function to draw a blob.  r is the mean radius and dri and dro are the inner 
 * and outer radii which are calculated as ri = r + dri.  ca and sa are the cosine
 * and sine of the mean angle of the blob respectively */
void DrawBlob(float r, float dri, float dro, float arc_degrees, float ca, float sa)
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

    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glEnableVertexAttribArray ( 0 );
    glDrawArrays( GL_TRIANGLES, 0, 3 );
    glDrawArrays( GL_TRIANGLES, 1, 3 );
}


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
    glEnableVertexAttribArray ( 0 );
    glDrawArrays( GL_LINES, 0, nVerts );
}

void DrawCircle(float radius)
{
    int i=0;
    int nVerts = 100;

    GLfloat *rVertices = malloc ( sizeof(GLfloat) * 3 * nVerts );
    for (i=0; i<nVerts; i++)
    {
        rVertices[(i*3)+0] = radius*sin(2.0*PI*(float)i/(float)nVerts);
        rVertices[(i*3)+1] = radius*cos(2.0*PI*(float)i/(float)nVerts);
        rVertices[(i*3)+2] = 0.0f;
    };

    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, rVertices);
    glEnableVertexAttribArray ( 0 );
    glDrawArrays( GL_LINE_LOOP, 0, nVerts );
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

    eglQuerySurface(esContext->eglDisplay, esContext->eglSurface, EGL_WIDTH, &width);
    eglQuerySurface(esContext->eglDisplay, esContext->eglSurface, EGL_HEIGHT, &height);
    vpsize = width;
    if (width > height)
        vpsize = height;

   UserData *userData = esContext->userData;
      
   // Set the viewport
   glViewport ( (width/2)-(vpsize/2), (height/2)-(vpsize/2), vpsize, vpsize );

   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( userData->programObject );

    // Draw the range circles
    DrawCircle(1.0);
    DrawCircle(0.33333);
    DrawCircle(0.66666);

    // Draw the crosshairs
    for (i=0; i<4; i++) { DrawTick(360.0*(float)i/4.0, 1.0, 0.0); };

    /* Draw the ticks */ 
    for (i=0; i<360; i++) 
    {
        if ( fmod(i, 10) == 0 ) { DrawTick(i, 1.0, 0.95); }
        else if ( fmod(i, 5) == 0 ) { DrawTick(i, 1.0, 0.97); }
        else { DrawTick(i, 1.0, 0.98); };
    };

    /* Draw a test blob */
    DrawBlob(0.5, 0.0, 0.1, 1, cos(22.5*PI/180.0), sin(22.5*PI/180.0));

   // Ping the radar
   pingRadar();
}


void *drawWindow(void *arg)
{
   ESContext esContext;
   UserData  userData;

   esInitContext ( &esContext );
   esContext.userData = &userData;

   esCreateWindow ( &esContext, "Hello Triangle", 320, 320, ES_WINDOW_RGB );

   if ( !Init ( &esContext ) )
      return 0;

   esRegisterDrawFunc ( &esContext, Draw );

   esMainLoop ( &esContext );
}

