#version 330 core

out vec4 FragColor;

in vec2 texCoord;

uniform vec2 panelDims;
uniform vec4 insideCol;
uniform vec4 borderCol;

const int thick = 2;

float DF(vec2 v)
{
	float corner = 16;
	float rad = max(thick , min(corner, min(panelDims.x, panelDims.y)/2));

	float dist = abs(corner - v.x) + abs(corner - v.y) - corner;
	dist = min(dist, abs(panelDims.x - corner - v.x) + abs(v.y - corner) - corner);
	dist = min(dist, abs(corner- v.x) + abs(panelDims.y - corner -  v.y) - corner);
	dist = min(dist, abs(panelDims.x - corner - v.x) + abs(panelDims.y - corner - v.y) - corner);

	float rectDist1 = max(v.x - panelDims.x, - v.x);
	rectDist1 = max(v.y - panelDims.y + rad, rad - v.y) > 0 ? 1 : rectDist1;
	float rectDist2 = max(v.y - panelDims.y, - v.y);
	rectDist2 = max(v.x - panelDims.x + rad, rad - v.x) > 0 ? 1 : rectDist2;

	return min(min(dist, rectDist1), rectDist2);
}

void main()
{
	vec2 winPos = texCoord * panelDims;
	float dist = DF(winPos);
	float coef = sin((winPos.x + winPos.y * 0.5) * 3.14159 / 100.0);

	vec4 col1 = vec4(borderCol.rgb * (1 - 0.25 * (coef + 1) * 0.5), borderCol.a);
	vec4 col2 = vec4( (1 - (texCoord.x * 0.5 + texCoord.y)/1.5 * 0.2) * insideCol.rgb, insideCol.a);
	FragColor = float(dist <= 0) * (dist < -thick ? col2 : col1 );
	if(FragColor.a <= 0) discard;

}

/*
const int thick = 3;
const int radius = 0;

float DF(vec2 v)
{
	float rad = max(thick , min(radius, min(panelDims.x, panelDims.y)/2));
	float dist = length(vec2(rad) - v) - rad;
	dist = min(dist, length(vec2(panelDims.x -rad, rad) - v) - rad);
	dist = min(dist, length(vec2(rad, panelDims.y -rad) - v) - rad);
	dist = min(dist, length(vec2(panelDims.x -rad, panelDims.y -rad) - v) - rad);

	float rectDist1 = max(v.x - panelDims.x, - v.x);
	rectDist1 = max(v.y - panelDims.y + rad, rad - v.y) > 0 ? 1 : rectDist1;
	float rectDist2 = max(v.y - panelDims.y, - v.y);
	rectDist2 = max(v.x - panelDims.x + rad, rad - v.x) > 0 ? 1 : rectDist2;

	
	return min(min(dist, rectDist1), rectDist2);
}

void main()
{
	vec4 insideCol = vec4(20, 30, 27, 255) / 255.f;
	vec4 borderCol1 = vec4(120,105, 75, 255) / 255.f;
	vec4 borderCol2 = vec4(40, 40, 40, 255) / 255.f;
	vec2 winPos = texCoord * panelDims;
	float dist = DF(winPos);
	float coef = sin((winPos.x + winPos.y * 0.5) * 3.14159 / 100.0);

	vec4 col1 = vec4(borderCol1.rgb * (1 + 0.15 * coef), borderCol1.a);
	vec4 col2 = vec4(borderCol2.rgb, borderCol2.a);
	vec4 col3 = vec4( (1 - (texCoord.x * 0.5 + texCoord.y)/1.5 * 0.2) * insideCol.rgb, insideCol.a);
	FragColor = float(dist <= 0) * (dist < -thick ? col3 : (dist < -thick * 0.5 ? col1 : col2) );

}*/
