#include "epch.h"
#include "Math.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/epsilon.hpp>

namespace Cosmos::Math
{
    bool Decompose(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
    {
        glm::mat4 localMatrix(transform);
        using T = float;

        // normalize the matrix
        if (glm::epsilonEqual(localMatrix[3][3], static_cast<T>(0), glm::epsilon<T>()))
            return false;

        // handle perspective
        bool row0 = glm::epsilonNotEqual(localMatrix[0][3], 0.0f, glm::epsilon<T>());
        bool row1 = glm::epsilonNotEqual(localMatrix[1][3], 0.0f, glm::epsilon<T>());
        bool row2 = glm::epsilonNotEqual(localMatrix[2][3], 0.0f, glm::epsilon<T>());

        if (row0 || row1 || row2)
        {
            localMatrix[0][3] = localMatrix[1][3] = localMatrix[2][3] = static_cast<T>(0);
            localMatrix[3][3] = static_cast<T>(1);
        }

        // handle translation
        translation = glm::vec3(localMatrix[3]);
        localMatrix[3] = glm::vec4(0, 0, 0, localMatrix[3].w);

        // handle scale
        glm::vec3 row[3], Pdum3;
        for (glm::length_t i = 0; i < 3; ++i)
            for (glm::length_t j = 0; j < 3; ++j)
                row[i][j] = localMatrix[i][j];

        // compute scale factor and normalize rows
        scale.x = glm::length(row[0]);
        row[0] = glm::detail::scale(row[0], static_cast<T>(1));
        scale.y = glm::length(row[1]);
        row[1] = glm::detail::scale(row[1], static_cast<T>(1));
        scale.z = glm::length(row[2]);
        row[2] = glm::detail::scale(row[2], static_cast<T>(1));

        // at this point, the matrix (in rows[]) is orthonormal.
        // check for a coordinate system flip. If the determinant is -1, then negate the matrix and the scaling factors.
#if 0
        Pdum3 = glm::cross(row[1], row[2]); // v3Cross(row[1], row[2], Pdum3);
        if (glm::dot(row[0], Pdum3) < 0)
        {
            for (glm::length_t i = 0; i < 3; i++)
            {
                scale[i] *= static_cast<T>(-1);
                row[i] *= static_cast<T>(-1);
            }
        }
#endif
        rotation.y = asin(-row[0][2]);

        if (cos(rotation.y) != 0)
        {
            rotation.x = atan2(row[1][2], row[2][2]);
            rotation.z = atan2(row[0][1], row[0][0]);
        }

        else
        {
            rotation.x = atan2(-row[2][0], row[1][1]);
            rotation.z = 0;
        }

        return true;
    }
}