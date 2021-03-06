// Copyright 2015-2019 Elviss Strazdins. All rights reserved.

#ifndef ANIMATIONSSAMPLE_HPP
#define ANIMATIONSSAMPLE_HPP

#include "ouzel.hpp"

class AnimationsSample: public ouzel::scene::Scene
{
public:
    AnimationsSample();

private:
    bool handleGamepad(const ouzel::GamepadEvent& event);
    bool handleUI(const ouzel::UIEvent& event) const;
    bool handleKeyboard(const ouzel::KeyboardEvent& event) const;

    ouzel::scene::Layer layer;
    ouzel::scene::Camera camera;
    ouzel::scene::Actor cameraActor;

    ouzel::scene::ShapeRenderer shapeDrawable;
    ouzel::scene::Actor drawActor;

    std::unique_ptr<ouzel::scene::Shake> shake;

    ouzel::scene::Sprite witchSprite;
    ouzel::scene::Actor witch;

    std::unique_ptr<ouzel::scene::Animator> witchScale;
    std::unique_ptr<ouzel::scene::Animator> witchFade;
    std::unique_ptr<ouzel::scene::Animator> witchRotate;
    std::unique_ptr<ouzel::scene::Animator> witchRepeat;
    std::unique_ptr<ouzel::scene::Animator> witchParallel;
    std::unique_ptr<ouzel::scene::Animator> witchSequence;

    ouzel::scene::Sprite ballSprite;
    ouzel::scene::Actor ball;

    std::unique_ptr<ouzel::scene::Animator> ballDelay;
    std::unique_ptr<ouzel::scene::Animator> ballEase;
    std::unique_ptr<ouzel::scene::Animator> ballMove;
    std::unique_ptr<ouzel::scene::Animator> ballSequence;

    ouzel::EventHandler handler;

    ouzel::scene::Layer guiLayer;
    ouzel::scene::Camera guiCamera;
    ouzel::scene::Actor guiCameraActor;
    ouzel::gui::Menu menu;
    ouzel::gui::Button backButton;
};

#endif // ANIMATIONSSAMPLE_HPP
