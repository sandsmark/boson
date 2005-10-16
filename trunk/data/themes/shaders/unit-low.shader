<vertex>

uniform vec3 lightPos;
uniform bool fogEnabled;

varying vec3 diffuse;
varying vec3 color;

vec3 eucleidian(vec4 h) { return vec3(h.xyz / h.w); }

void main()
{
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

  color = gl_Color.rgb;

  // Transform everything into eye space. We can't use untranformed stuff
  //  because models use matrix transformations.
  vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
  vec4 eyevertex = gl_ModelViewMatrix * gl_Vertex;
  vec3 vertex = eucleidian(eyevertex);
  vec3 tolight = normalize(eucleidian(gl_ModelViewMatrix * vec4(lightPos, 1.0)) - vertex);

  diffuse = dot(normal, tolight) * gl_LightSource[0].diffuse.rgb;


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

varying vec3 diffuse;
varying vec3 color;

void main()
{
  // Get diffuse texture color at this point
  vec4 texcolor = texture2D(texture_0, gl_TexCoord[0].xy);

  // Get shadow strength at this point
  float shadow = shadow2DProj(texture_3, gl_TexCoord[1]).r;

  vec3 litcolor = texcolor.rgb * (gl_LightSource[0].ambient.rgb + shadow * diffuse) * color;

  gl_FragColor = vec4(litcolor, texcolor.a);
}
