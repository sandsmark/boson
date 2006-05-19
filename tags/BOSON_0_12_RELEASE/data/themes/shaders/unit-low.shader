<vertex>

varying vec3 ambient;
varying vec3 diffuse;

void main()
{
  gl_TexCoord[0] = gl_MultiTexCoord0;

  // Transform everything into eye space. We can't use untranformed stuff
  //  because models use matrix transformations.
  vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
  vec4 eyevertex = gl_ModelViewMatrix * gl_Vertex;
  // FIXME: this assumes we're using directional light
  vec3 tolight = normalize(gl_LightSource[0].position.xyz);

  ambient = gl_LightSource[0].ambient.rgb * gl_Color.rgb;
  diffuse = dot(normal, tolight) * gl_LightSource[0].diffuse.rgb * gl_Color.rgb;


  gl_TexCoord[1].s = dot(eyevertex, gl_EyePlaneS[3]);
  gl_TexCoord[1].t = dot(eyevertex, gl_EyePlaneT[3]);
  gl_TexCoord[1].p = dot(eyevertex, gl_EyePlaneR[3]);
  gl_TexCoord[1].q = dot(eyevertex, gl_EyePlaneQ[3]);
  gl_TexCoord[1] = gl_TextureMatrix[3] * gl_TexCoord[1];

  gl_Position = ftransform();
}


<fragment>

uniform sampler2D texture_0;
uniform sampler2DShadow texture_3;

varying vec3 ambient;
varying vec3 diffuse;

void main()
{
  // Get diffuse texture color at this point
  vec4 texcolor = texture2D(texture_0, gl_TexCoord[0].xy);

  // Get shadow strength at this point
  float shadow = shadow2DProj(texture_3, gl_TexCoord[1]).r;

  gl_FragColor = vec4(texcolor.rgb * (ambient + shadow * diffuse), texcolor.a);
}
