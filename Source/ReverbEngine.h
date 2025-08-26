#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>

/**
 * @brief Simplified reverb engine for initial testing
 * 
 * A basic but functional reverb implementation that avoids
 * complex dependencies while providing good sound quality.
 */
class ReverbEngine
{
public:
    ReverbEngine();
    ~ReverbEngine() = default;
    
    /**
     * @brief Prepare the reverb engine for processing
     */
    void prepare(double sampleRate, int expectedBlockSize, int numChannels);
    
    /**
     * @brief Reset all internal states
     */
    void reset();
    
    /**
     * @brief Process an audio buffer
     */
    void processBlock(juce::AudioBuffer<float>& buffer);
    
    // Parameter setters
    void setRoomSize(float roomSize);
    void setDecayTime(float decayTime);
    void setPreDelay(float preDelayMs);
    void setDamping(float damping);
    void setWetDryMix(float wetDryMix);
    void setEarlyLateBalance(float balance);
    void setDiffusion(float diffusion);
    void setModulationRate(float rate);
    void setModulationDepth(float depth);
    
    // Parameter getters
    float getRoomSize() const { return m_roomSize; }
    float getDecayTime() const { return m_decayTime; }
    float getPreDelay() const { return m_preDelayMs; }
    float getDamping() const { return m_damping; }
    float getWetDryMix() const { return m_wetDryMix; }
    float getEarlyLateBalance() const { return m_earlyLateBalance; }
    float getDiffusion() const { return m_diffusion; }
    float getModulationRate() const { return m_modulationRate; }
    float getModulationDepth() const { return m_modulationDepth; }
    
private:
    // Simple delay line implementation
    class SimpleDelay {
    public:
        void setSize(int samples) {
            buffer.resize(samples);
            std::fill(buffer.begin(), buffer.end(), 0.0f);
            index = 0;
        }
        
        float process(float input) {
            if (buffer.empty()) return input;
            float output = buffer[index];
            buffer[index] = input;
            index = (index + 1) % buffer.size();
            return output;
        }
        
        void clear() {
            std::fill(buffer.begin(), buffer.end(), 0.0f);
        }
        
    private:
        std::vector<float> buffer;
        int index = 0;
    };
    
    // Comb filter for reverb tail
    class SimpleComb {
    public:
        void setSize(int samples) {
            delay.setSize(samples);
            feedback = 0.7f;
        }
        
        void setFeedback(float fb) {
            feedback = juce::jlimit(0.0f, 0.95f, fb);
        }
        
        float process(float input) {
            float delayed = delay.process(input + lastOutput * feedback);
            lastOutput = delayed;
            return delayed;
        }
        
        void clear() {
            delay.clear();
            lastOutput = 0.0f;
        }
        
    private:
        SimpleDelay delay;
        float feedback = 0.7f;
        float lastOutput = 0.0f;
    };
    
    // Parameters
    float m_roomSize;
    float m_decayTime;
    float m_preDelayMs;
    float m_damping;
    float m_wetDryMix;
    float m_earlyLateBalance;
    float m_diffusion;
    float m_modulationRate;
    float m_modulationDepth;
    
    // Internal state
    double m_sampleRate;
    int m_numChannels;
    bool m_isPrepared;
    
    // Processing components
    SimpleDelay m_preDelay;
    std::vector<SimpleComb> m_combFilters;
    
    // Early reflections delays
    std::vector<SimpleDelay> m_earlyDelays;
    std::vector<float> m_earlyGains;
    
    /**
     * @brief Update internal parameters
     */
    void updateParameters();
    
    /**
     * @brief Process stereo samples
     */
    void processStereo(float leftInput, float rightInput, 
                      float& leftOutput, float& rightOutput);
};