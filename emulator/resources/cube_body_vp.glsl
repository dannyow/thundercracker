varying vec3 normal;

void main() {
     normal = gl_Normal * gl_NormalMatrix;
     gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}