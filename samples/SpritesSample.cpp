// Copyright 2015-2019 Elviss Strazdins. All rights reserved.

#include "SpritesSample.hpp"
#include "MainMenu.hpp"

using namespace ouzel;
using namespace input;

SpritesSample::SpritesSample():
    hideButton("button.png", "button_selected.png", "button_down.png", "", "Show/hide", "Arial", 1.0F, Color::BLACK, Color::BLACK, Color::BLACK),
    wireframeButton("button.png", "button_selected.png", "button_down.png", "", "Wireframe", "Arial", 1.0F, Color::BLACK, Color::BLACK, Color::BLACK),
    backButton("button.png", "button_selected.png", "button_down.png", "", "Back", "Arial", 1.0F, Color::BLACK, Color::BLACK, Color::BLACK)
{
    handler.gamepadHandler = std::bind(&SpritesSample::handleGamepad, this, std::placeholders::_1);
    handler.uiHandler = std::bind(&SpritesSample::handleUI, this, std::placeholders::_1);
    handler.keyboardHandler = std::bind(&SpritesSample::handleKeyboard, this, std::placeholders::_1);
    engine->getEventDispatcher().addEventHandler(&handler);

    camera.setClearColorBuffer(true);
    camera.setClearColor(ouzel::Color(64, 64, 64));
    camera.setScaleMode(scene::Camera::ScaleMode::SHOW_ALL);
    camera.setTargetContentSize(Size2F(800.0F, 600.0F));
    cameraActor.addComponent(&camera);
    layer.addChild(&cameraActor);
    addLayer(&layer);

    // character
    characterSprite.init("run.json");
    characterSprite.setAnimation("", true);
    characterSprite.play();

    character.addComponent(&characterSprite);
    layer.addChild(&character);
    character.setPosition(Vector2F(-300.0F, 0.0F));

    move.reset(new scene::Move(4.0F, Vector3F(300.0F, 0.0F, 0.0F)));
    character.addComponent(move.get());
    move->start();

    // fire
    fireSprite.init("fire.json");
    fireSprite.setOffset(Vector2F(0.0F, 20.0F));
    fireSprite.setAnimation("", true);
    fireSprite.play();

    fireActor.addComponent(&fireSprite);
    fireActor.setPosition(Vector2F(-100.0F, -140.0F));
    layer.addChild(&fireActor);

    // triangle
    triangleSprite.init("triangle.json");

    triangleActor.addComponent(&triangleSprite);
    triangleActor.setPosition(Vector2F(100.0F, -140.0F));
    layer.addChild(&triangleActor);

    guiCamera.setScaleMode(scene::Camera::ScaleMode::SHOW_ALL);
    guiCamera.setTargetContentSize(Size2F(800.0F, 600.0F));
    guiCameraActor.addComponent(&guiCamera);
    guiLayer.addChild(&guiCameraActor);
    addLayer(&guiLayer);

    guiLayer.addChild(&menu);

    hideButton.setPosition(Vector2F(-200.0F, 200.0F));
    menu.addWidget(&hideButton);

    wireframeButton.setPosition(Vector2F(-200.0F, 160.0F));
    menu.addWidget(&wireframeButton);

    backButton.setPosition(Vector2F(-200.0F, -200.0F));
    menu.addWidget(&backButton);
}

bool SpritesSample::handleGamepad(const GamepadEvent& event)
{
    if (event.type == Event::Type::GAMEPAD_BUTTON_CHANGE)
    {
        if (event.pressed &&
            event.button == Gamepad::Button::FACE_RIGHT)
            engine->getSceneManager().setScene(std::unique_ptr<scene::Scene>(new MainMenu()));
    }

    return false;
}

bool SpritesSample::handleUI(const UIEvent& event)
{
    if (event.type == Event::Type::ACTOR_CLICK)
    {
        if (event.actor == &backButton)
            engine->getSceneManager().setScene(std::unique_ptr<scene::Scene>(new MainMenu()));
        else if (event.actor == &hideButton)
            character.setHidden(!character.isHidden());
        else if (event.actor == &wireframeButton)
            camera.setWireframe(!camera.getWireframe());
    }

    return false;
}

bool SpritesSample::handleKeyboard(const KeyboardEvent& event) const
{
    if (event.type == Event::Type::KEY_PRESS)
    {
        switch (event.key)
        {
            case Keyboard::Key::ESCAPE:
            case Keyboard::Key::MENU:
            case Keyboard::Key::BACK:
                engine->getSceneManager().setScene(std::unique_ptr<scene::Scene>(new MainMenu()));
                return true;
            default:
                break;
        }
    }
    else if (event.type == Event::Type::KEY_RELEASE)
    {
        switch (event.key)
        {
            case Keyboard::Key::ESCAPE:
            case Keyboard::Key::MENU:
            case Keyboard::Key::BACK:
                return true;
            default:
                break;
        }
    }

    return false;
}
