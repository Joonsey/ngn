#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D uUvMap;
uniform int uvmapwidth;
uniform int uvmapheight;

// Output fragment color
out vec4 finalColor;

void main()
{
    // frag color from the primary texture (entity usually)
    vec4 fragColor = texture(texture0, fragTexCoord);

    // Sample color from the UV map texture
    vec4 uvColor = texture(uUvMap, vec2(fragColor.r * 255 / uvmapwidth, fragColor.g * 255 / uvmapheight));

    // Use the sampled UV color as the final fragment color
    finalColor = uvColor;
}
