#version 330 core

uniform float alpha;

in vec4 m_color;
in float m_depth;
out vec4 FragColor;

void main(void) {
    FragColor = m_color;
    FragColor.a = alpha * (0.3 - min(m_color.a * (m_depth * m_depth) * 0.0001, 0.3));
}