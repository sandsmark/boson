<vertex>

uniform vec3 lightPos;

varying float alpha;
varying vec3 diffuselight;

void main()
{
  // Stuff that will be sent to fragment shader
  alpha = gl_Color.w;

  // Texture coordinate for accessing diffuse and normalmap textures
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_Vertex;

  // Texture coordinate for accessing fog texture
  gl_TexCoord[1] = gl_TextureMatrix[1] * gl_Vertex;
  // And for shadowmap texture
  gl_TexCoord[3] = gl_TextureMatrix[3] * gl_Vertex;

  diffuselight = dot(gl_Normal, normalize(lightPos - gl_Vertex.xyz)) * gl_LightSource[0].diffuse.rgb;

  gl_Position = ftransform();
}


<fragment>

// Diffuse map
uniform sampler2D texture_0;
// Fog map
uniform sampler2D texture_1;
// Shadowmap
uniform sampler2DShadow texture_3;

varying float alpha;
varying vec3 diffuselight;

void main()
{
  // Don't render invisible terrain
  if(alpha == 0.0)
    discard;

  float visibility = texture2D(texture_1, gl_TexCoord[1].xy).r;
  // Don't render fogged terrain
  if(visibility == 0.0)
    discard;

  // Color from the diffuse texture
  vec3 basetexcolor = texture2D(texture_0, gl_TexCoord[0].xy).rgb;

  float shadow = shadow2DProj(texture_3, gl_TexCoord[3]).r;

  vec3 litcolor = basetexcolor * (diffuselight * shadow + gl_LightSource[0].ambient.rgb);

  gl_FragColor = vec4(litcolor * visibility, alpha);
}

