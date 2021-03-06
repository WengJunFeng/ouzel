// Copyright 2015-2019 Elviss Strazdins. All rights reserved.

#include <cmath>
#include "Effects.hpp"
#include "Audio.hpp"
#include "scene/Actor.hpp"
#include "math/MathUtils.hpp"
#include "smbPitchShift.hpp"

namespace ouzel
{
    namespace audio
    {
        class DelayProcessor final: public mixer::Processor
        {
        public:
            DelayProcessor(float initDelay):
                delay(initDelay)
            {
            }

            void process(uint32_t frames, uint16_t channels, uint32_t sampleRate,
                         std::vector<float>& samples) override
            {
                uint32_t delayFrames = static_cast<uint32_t>(delay * sampleRate);
                uint32_t bufferFrames = frames + delayFrames;

                buffer.resize(bufferFrames * channels);

                for (uint16_t channel = 0; channel < channels; ++channel)
                {
                    float* bufferChannel = &buffer[channel * bufferFrames];
                    float* outputChannel = &samples[channel * frames];

                    for (uint32_t frame = 0; frame < frames; ++frame)
                        bufferChannel[frame + delayFrames] += outputChannel[frame];

                    for (uint32_t frame = 0; frame < frames; ++frame)
                        outputChannel[frame] = bufferChannel[frame];

                    // erase frames from beginning
                    for (uint32_t frame = 0; frame < bufferFrames - frames; ++frame)
                        bufferChannel[frame] = bufferChannel[frame + frames];

                    for (uint32_t frame = bufferFrames - frames; frame < bufferFrames; ++frame)
                        bufferChannel[frame] = 0.0F;
                }
            }

            void setDelay(float newDelay)
            {
                delay = newDelay;
            }

        private:
            float delay = 0.0F;
            std::vector<float> buffer;
        };

        Delay::Delay(Audio& initAudio, float initDelay):
            Effect(initAudio,
                   initAudio.initProcessor(std::unique_ptr<mixer::Processor>(new DelayProcessor(initDelay)))),
            delay(initDelay)
        {
        }

        Delay::~Delay()
        {
        }

        void Delay::setDelay(float newDelay)
        {
            delay = newDelay;

            audio.updateProcessor(processorId, [newDelay](mixer::Object* node) {
                DelayProcessor* delayProcessor = static_cast<DelayProcessor*>(node);
                delayProcessor->setDelay(newDelay);
            });
        }

        class GainProcessor final: public mixer::Processor
        {
        public:
            GainProcessor(float initGain = 0.0F):
                gain(initGain),
                gainFactor(pow(10.0F, initGain / 20.0F))
            {
            }

            void process(uint32_t, uint16_t, uint32_t,
                         std::vector<float>& samples) override
            {
                for (float& sample : samples)
                    sample *= gainFactor;
            }

            void setGain(float newGain)
            {
                gain = newGain;
                gainFactor = pow(10.0F, gain / 20.0F);
            }

        private:
            float gain = 0.0F;
            float gainFactor = 1.0F;
        };

        Gain::Gain(Audio& initAudio, float initGain):
            Effect(initAudio,
                   initAudio.initProcessor(std::unique_ptr<mixer::Processor>(new GainProcessor(initGain)))),
            gain(initGain)
        {
        }

        Gain::~Gain()
        {
        }

        void Gain::setGain(float newGain)
        {
            gain = newGain;

            audio.updateProcessor(processorId, [newGain](mixer::Object* node) {
                GainProcessor* gainProcessor = static_cast<GainProcessor*>(node);
                gainProcessor->setGain(newGain);
            });
        }

        class PannerProcessor final: public mixer::Processor
        {
        public:
            PannerProcessor()
            {
            }

            void process(uint32_t frames, uint16_t channels, uint32_t sampleRate,
                         std::vector<float>& samples) override
            {
            }

            void setPosition(const Vector3F& newPosition)
            {
                position = newPosition;
            }

            void setRolloffFactor(float newRolloffFactor)
            {
                rolloffFactor = newRolloffFactor;
            }

            void setMinDistance(float newMinDistance)
            {
                minDistance = newMinDistance;
            }

            void setMaxDistance(float newMaxDistance)
            {
                maxDistance = newMaxDistance;
            }

        private:
            Vector3F position;
            float rolloffFactor = 1.0F;
            float minDistance = 1.0F;
            float maxDistance = FLT_MAX;
        };

        Panner::Panner(Audio& initAudio):
            Effect(initAudio,
                   initAudio.initProcessor(std::unique_ptr<mixer::Processor>(new PannerProcessor()))),
            scene::Component(scene::Component::SOUND)
        {
        }

        Panner::~Panner()
        {
        }

        void Panner::setPosition(const Vector3F& newPosition)
        {
            position = newPosition;

            audio.updateProcessor(processorId, [newPosition](mixer::Object* node) {
                PannerProcessor* pannerProcessor = static_cast<PannerProcessor*>(node);
                pannerProcessor->setPosition(newPosition);
            });
        }

        void Panner::setRolloffFactor(float newRolloffFactor)
        {
            rolloffFactor = newRolloffFactor;

            audio.updateProcessor(processorId, [newRolloffFactor](mixer::Object* node) {
                PannerProcessor* pannerProcessor = static_cast<PannerProcessor*>(node);
                pannerProcessor->setRolloffFactor(newRolloffFactor);
            });
        }

        void Panner::setMinDistance(float newMinDistance)
        {
            minDistance = newMinDistance;

            audio.updateProcessor(processorId, [newMinDistance](mixer::Object* node) {
                PannerProcessor* pannerProcessor = static_cast<PannerProcessor*>(node);
                pannerProcessor->setMinDistance(newMinDistance);
            });
        }

        void Panner::setMaxDistance(float newMaxDistance)
        {
            maxDistance = newMaxDistance;

            audio.updateProcessor(processorId, [newMaxDistance](mixer::Object* node) {
                PannerProcessor* pannerProcessor = static_cast<PannerProcessor*>(node);
                pannerProcessor->setMaxDistance(newMaxDistance);
            });
        }

        void Panner::updateTransform()
        {
            setPosition(actor->getWorldPosition());
        }

        static constexpr float MIN_PITCH = 0.5F;
        static constexpr float MAX_PITCH = 2.0F;

        class PitchProcessor final: public mixer::Processor
        {
        public:
            PitchProcessor(float initPitch):
                pitch(clamp(initPitch, MIN_PITCH, MAX_PITCH))
            {
            }

            void process(uint32_t frames, uint16_t channels, uint32_t sampleRate,
                         std::vector<float>& samples) override
            {
                channelSamples.resize(frames);
                pitchShift.resize(channels);

                for (uint16_t channel = 0; channel < channels; ++channel)
                {
                    pitchShift[channel].process(pitch, frames, 1024, 4, static_cast<float>(sampleRate),
                                                samples.data() + channel * frames,
                                                channelSamples.data());

                    float* outputChannel = &samples[channel * frames];

                    for (uint32_t frame = 0; frame < frames; ++frame)
                        outputChannel[frame] = channelSamples[frame];
                }
            }

            void setPitch(float newPitch)
            {
                pitch = clamp(newPitch, MIN_PITCH, MAX_PITCH);
            }

        private:
            float pitch = 1.0f;
            std::vector<float> channelSamples;
            std::vector<smb::PitchShift> pitchShift;
        };

        Pitch::Pitch(Audio& initAudio, float initPitch):
            Effect(initAudio,
                   initAudio.initProcessor(std::unique_ptr<mixer::Processor>(new PitchProcessor(initPitch)))),
            pitch(initPitch)
        {
        }

        Pitch::~Pitch()
        {
        }

        void Pitch::setPitch(float newPitch)
        {
            pitch = newPitch;

            audio.updateProcessor(processorId, [newPitch](mixer::Object* node) {
                PitchProcessor* pitchProcessor = static_cast<PitchProcessor*>(node);
                pitchProcessor->setPitch(newPitch);
            });
        }

        class ReverbProcessor final: public mixer::Processor
        {
        public:
            ReverbProcessor(float initDelay, float initDecay):
                delay(initDelay), decay(initDecay)
            {
            }

            void process(uint32_t frames, uint16_t channels, uint32_t sampleRate,
                         std::vector<float>& samples) override
            {
                uint32_t delayFrames = static_cast<uint32_t>(delay * sampleRate);
                uint32_t bufferFrames = frames + delayFrames;

                buffer.resize(bufferFrames * channels);

                for (uint16_t channel = 0; channel < channels; ++channel)
                {
                    float* bufferChannel = &buffer[channel * bufferFrames];
                    float* outputChannel = &samples[channel * frames];

                    for (uint32_t frame = 0; frame < frames; ++frame)
                        bufferChannel[frame] += outputChannel[frame];

                    for (uint32_t frame = 0; frame < frames; ++frame)
                        bufferChannel[frame + delayFrames] += bufferChannel[frame] * decay;

                    for (uint32_t frame = 0; frame < frames; ++frame)
                        outputChannel[frame] = bufferChannel[frame];

                    // erase frames from beginning
                    for (uint32_t frame = 0; frame < delayFrames; ++frame)
                        bufferChannel[frame] = bufferChannel[frame + frames];

                    for (uint32_t frame = delayFrames; frame < bufferFrames; ++frame)
                        bufferChannel[frame] = 0.0F;
                }
            }

        private:
            float delay = 0.1F;
            float decay = 0.5F;
            std::vector<float> buffer;
        };

        Reverb::Reverb(Audio& initAudio, float initDelay, float initDecay):
            Effect(initAudio,
                   initAudio.initProcessor(std::unique_ptr<mixer::Processor>(new ReverbProcessor(initDelay, initDecay)))),
            delay(initDelay),
            decay(initDecay)
        {
        }

        Reverb::~Reverb()
        {
        }

        LowPass::LowPass(Audio& initAudio):
            Effect(initAudio,
                   initAudio.initProcessor(std::unique_ptr<mixer::Processor>()))
        {
        }

        LowPass::~LowPass()
        {
        }

        HighPass::HighPass(Audio& initAudio):
            Effect(initAudio,
                   initAudio.initProcessor(std::unique_ptr<mixer::Processor>()))
        {
        }

        HighPass::~HighPass()
        {
        }
    } // namespace audio
} // namespace ouzel
