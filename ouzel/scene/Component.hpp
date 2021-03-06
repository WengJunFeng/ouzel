// Copyright 2015-2019 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_SCENE_COMPONENT_HPP
#define OUZEL_SCENE_COMPONENT_HPP

#include <memory>
#include <vector>
#include "math/Box.hpp"
#include "math/Matrix.hpp"
#include "math/Color.hpp"
#include "math/Rect.hpp"
#include "graphics/Texture.hpp"

namespace ouzel
{
    namespace scene
    {
        class Actor;
        class Layer;

        class Component
        {
            friend Actor;
        public:
            enum Class
            {
                NONE = 0,
                CAMERA = 1,
                ANIMATOR = 2,
                MODEL_RENDERER = 3,
                PARTICLE_SYSTEM = 4,
                SHAPE_RENDERER = 5,
                LISTENER = 6,
                SOUND = 7,
                SPRITE = 8,
                TEXT_RENDERER = 9,
                LIGHT = 10
            };

            explicit Component(uint32_t initClass);
            virtual ~Component();

            Component(const Component&) = delete;
            Component& operator=(const Component&) = delete;

            Component(Component&&) = delete;
            Component& operator=(Component&&) = delete;

            inline uint32_t getClass() const { return cls; }

            virtual void draw(const Matrix4F& transformMatrix,
                              float opacity,
                              const Matrix4F& renderViewProjection,
                              bool wireframe);

            virtual const Box3F& getBoundingBox() const { return boundingBox; }
            virtual void setBoundingBox(const Box3F& newBoundingBox) { boundingBox = newBoundingBox; }

            virtual bool pointOn(const Vector2F& position) const;
            virtual bool shapeOverlaps(const std::vector<Vector2F>& edges) const;

            inline bool isHidden() const { return hidden; }
            inline void setHidden(bool newHidden) { hidden = newHidden; }

            inline Actor* getActor() const { return actor; }
            void removeFromActor();

        protected:
            virtual void setActor(Actor* newActor);
            virtual void setLayer(Layer* newLayer);
            virtual void updateTransform();

            uint32_t cls;

            Box3F boundingBox;
            bool hidden = false;

            Layer* layer = nullptr;
            Actor* actor = nullptr;
        };
    } // namespace scene
} // namespace ouzel

#endif // OUZEL_SCENE_COMPONENT_HPP
