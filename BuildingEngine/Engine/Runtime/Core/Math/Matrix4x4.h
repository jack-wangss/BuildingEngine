#pragma once


namespace BE
{
    class Matrix4x4
    {
    public:
        /// The matrix entries, indexed by [row][col]
        float m_mat[4][4];

        Matrix4x4() { operator=(IDENTITY); }

        Matrix4x4(float m00,
            float m01,
            float m02,
            float m03,
            float m10,
            float m11,
            float m12,
            float m13,
            float m20,
            float m21,
            float m22,
            float m23,
            float m30,
            float m31,
            float m32,
            float m33)
        {
            m_mat[0][0] = m00;
            m_mat[0][1] = m01;
            m_mat[0][2] = m02;
            m_mat[0][3] = m03;
            m_mat[1][0] = m10;
            m_mat[1][1] = m11;
            m_mat[1][2] = m12;
            m_mat[1][3] = m13;
            m_mat[2][0] = m20;
            m_mat[2][1] = m21;
            m_mat[2][2] = m22;
            m_mat[2][3] = m23;
            m_mat[3][0] = m30;
            m_mat[3][1] = m31;
            m_mat[3][2] = m32;
            m_mat[3][3] = m33;
        }

        static const Matrix4x4 IDENTITY;
    };
}