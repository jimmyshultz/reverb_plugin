#include "Modulator.h"
#include <cmath>
#include <algorithm>

// LFO implementation
LFO::LFO()
    : m_frequency(1.0f)
    , m_phase(0.0f)
    , m_phaseIncrement(0.0f)
    , m_waveform(Waveform::Sine)
    , m_sampleRate(44100.0)
    , m_noiseValue(0.0f)
    , m_noiseSampleCounter(0)
{
}

void LFO::prepare(double sampleRate)
{
    m_sampleRate = sampleRate;
    updatePhaseIncrement();
}

void LFO::reset()
{
    m_phase = 0.0f;
    m_noiseValue = 0.0f;
    m_noiseSampleCounter = 0;
}

void LFO::setFrequency(float frequency)
{
    m_frequency = juce::jlimit(0.01f, 20.0f, frequency);
    updatePhaseIncrement();
}

void LFO::setWaveform(Waveform waveform)
{
    m_waveform = waveform;
}

void LFO::setPhase(float phase)
{
    m_phase = std::fmod(phase, 2.0f * juce::MathConstants<float>::pi);
}

float LFO::getNextSample()
{
    float output = generateWaveform(m_phase);
    
    // Advance phase
    m_phase += m_phaseIncrement;
    if (m_phase >= 2.0f * juce::MathConstants<float>::pi)
        m_phase -= 2.0f * juce::MathConstants<float>::pi;
    
    return output;
}

void LFO::updatePhaseIncrement()
{
    m_phaseIncrement = 2.0f * juce::MathConstants<float>::pi * m_frequency / static_cast<float>(m_sampleRate);
}

float LFO::generateWaveform(float phase) const
{
    switch (m_waveform)
    {
        case Waveform::Sine:
            return std::sin(phase);
            
        case Waveform::Triangle:
        {
            float normalizedPhase = phase / (2.0f * juce::MathConstants<float>::pi);
            if (normalizedPhase < 0.5f)
                return 4.0f * normalizedPhase - 1.0f;
            else
                return 3.0f - 4.0f * normalizedPhase;
        }
        
        case Waveform::Sawtooth:
        {
            float normalizedPhase = phase / (2.0f * juce::MathConstants<float>::pi);
            return 2.0f * normalizedPhase - 1.0f;
        }
        
        case Waveform::Square:
            return (phase < juce::MathConstants<float>::pi) ? 1.0f : -1.0f;
            
        case Waveform::Noise:
        {
            // Update noise value periodically for smooth changes
            if (m_noiseSampleCounter++ >= NOISE_UPDATE_RATE)
            {
                m_noiseValue = m_random.nextFloat() * 2.0f - 1.0f;
                m_noiseSampleCounter = 0;
            }
            return m_noiseValue;
        }
        
        default:
            return 0.0f;
    }
}

// MultiLFO implementation
MultiLFO::MultiLFO()
    : m_sampleRate(44100.0)
{
}

void MultiLFO::prepare(double sampleRate)
{
    m_sampleRate = sampleRate;
    
    for (auto& lfo : m_lfos)
    {
        lfo->prepare(sampleRate);
    }
}

void MultiLFO::reset()
{
    for (auto& lfo : m_lfos)
    {
        lfo->reset();
    }
}

void MultiLFO::configure(const std::vector<float>& frequencies,
                        const std::vector<LFO::Waveform>& waveforms,
                        const std::vector<float>& phases,
                        const std::vector<float>& weights)
{
    // Ensure all vectors have the same size
    size_t numLFOs = std::min({frequencies.size(), waveforms.size(), phases.size(), weights.size()});
    numLFOs = std::min(numLFOs, MAX_LFOS);
    
    // Resize arrays
    m_lfos.clear();
    m_weights.clear();
    
    m_lfos.reserve(numLFOs);
    m_weights.reserve(numLFOs);
    
    // Create and configure LFOs
    for (size_t i = 0; i < numLFOs; ++i)
    {
        m_lfos.push_back(std::make_unique<LFO>());
        m_lfos[i]->prepare(m_sampleRate);
        m_lfos[i]->setFrequency(frequencies[i]);
        m_lfos[i]->setWaveform(waveforms[i]);
        m_lfos[i]->setPhase(phases[i]);
        
        m_weights.push_back(weights[i]);
    }
}

float MultiLFO::getNextSample()
{
    float output = 0.0f;
    float totalWeight = 0.0f;
    
    for (size_t i = 0; i < m_lfos.size(); ++i)
    {
        output += m_lfos[i]->getNextSample() * m_weights[i];
        totalWeight += std::abs(m_weights[i]);
    }
    
    // Normalize by total weight to prevent clipping
    if (totalWeight > 0.0f)
        output /= totalWeight;
    
    return juce::jlimit(-1.0f, 1.0f, output);
}

float MultiLFO::getLFOOutput(size_t index)
{
    if (index < m_lfos.size())
        return m_lfos[index]->getNextSample();
    
    return 0.0f;
}

// ModulationMatrix implementation
ModulationMatrix::ModulationMatrix()
    : m_envelopeValue(0.0f)
{
    // Initialize destination values to zero
    std::fill(m_destinationValues.begin(), m_destinationValues.end(), 0.0f);
}

void ModulationMatrix::prepare(double sampleRate)
{
    m_multiLFO.prepare(sampleRate);
    
    // Configure default LFOs
    std::vector<float> frequencies = { 0.5f, 1.2f, 2.3f, 0.8f };
    std::vector<LFO::Waveform> waveforms = { 
        LFO::Waveform::Sine, 
        LFO::Waveform::Triangle, 
        LFO::Waveform::Sine, 
        LFO::Waveform::Noise 
    };
    std::vector<float> phases = { 0.0f, juce::MathConstants<float>::pi * 0.5f, 
                                 juce::MathConstants<float>::pi, 0.0f };
    std::vector<float> weights = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    m_multiLFO.configure(frequencies, waveforms, phases, weights);
}

void ModulationMatrix::reset()
{
    m_multiLFO.reset();
    std::fill(m_destinationValues.begin(), m_destinationValues.end(), 0.0f);
}

void ModulationMatrix::setRoute(const ModulationRoute& route)
{
    // Find existing route or add new one
    auto it = std::find_if(m_routes.begin(), m_routes.end(),
        [&route](const ModulationRoute& r) {
            return r.source == route.source && r.destination == route.destination;
        });
    
    if (it != m_routes.end())
    {
        *it = route;
    }
    else
    {
        m_routes.push_back(route);
    }
}

void ModulationMatrix::removeRoute(Source source, Destination destination)
{
    m_routes.erase(
        std::remove_if(m_routes.begin(), m_routes.end(),
            [source, destination](const ModulationRoute& route) {
                return route.source == source && route.destination == destination;
            }),
        m_routes.end());
}

void ModulationMatrix::processSample()
{
    // Update LFOs
    m_multiLFO.getNextSample();
    
    // Update envelope (simplified - could be more complex)
    m_envelopeValue = 1.0f; // Constant for now
    
    // Update all destination values
    updateDestinations();
}

float ModulationMatrix::getModulationValue(Destination destination) const
{
    size_t index = static_cast<size_t>(destination);
    if (index < m_destinationValues.size())
        return m_destinationValues[index];
    
    return 0.0f;
}

void ModulationMatrix::updateDestinations()
{
    // Reset destination values
    std::fill(m_destinationValues.begin(), m_destinationValues.end(), 0.0f);
    
    // Process all active routes
    for (const auto& route : m_routes)
    {
        if (!route.enabled)
            continue;
        
        float sourceValue = getSourceValue(route.source);
        size_t destIndex = static_cast<size_t>(route.destination);
        
        if (destIndex < m_destinationValues.size())
        {
            m_destinationValues[destIndex] += sourceValue * route.amount;
        }
    }
    
    // Clamp destination values
    for (auto& value : m_destinationValues)
    {
        value = juce::jlimit(-1.0f, 1.0f, value);
    }
}

float ModulationMatrix::getSourceValue(Source source) const
{
    switch (source)
    {
        case Source::LFO1:
            return (m_multiLFO.getNumLFOs() > 0) ? m_multiLFO.getLFOOutput(0) : 0.0f;
        case Source::LFO2:
            return (m_multiLFO.getNumLFOs() > 1) ? m_multiLFO.getLFOOutput(1) : 0.0f;
        case Source::LFO3:
            return (m_multiLFO.getNumLFOs() > 2) ? m_multiLFO.getLFOOutput(2) : 0.0f;
        case Source::LFO4:
            return (m_multiLFO.getNumLFOs() > 3) ? m_multiLFO.getLFOOutput(3) : 0.0f;
        case Source::Envelope:
            return m_envelopeValue;
        case Source::Random:
            return m_random.nextFloat() * 2.0f - 1.0f;
        default:
            return 0.0f;
    }
}

// PitchModulator implementation
PitchModulator::PitchModulator()
    : m_rate(1.0f)
    , m_depth(10.0f) // 10 cents
    , m_sampleRate(44100.0)
    , m_writeIndex(0)
    , m_maxDelayInSamples(0)
{
}

void PitchModulator::prepare(double sampleRate, int maxDelayInSamples)
{
    m_sampleRate = sampleRate;
    m_maxDelayInSamples = maxDelayInSamples;
    
    m_delayBuffer.resize(maxDelayInSamples);
    std::fill(m_delayBuffer.begin(), m_delayBuffer.end(), 0.0f);
    
    m_lfo.prepare(sampleRate);
    m_lfo.setWaveform(LFO::Waveform::Sine);
    m_lfo.setFrequency(m_rate);
}

void PitchModulator::reset()
{
    std::fill(m_delayBuffer.begin(), m_delayBuffer.end(), 0.0f);
    m_writeIndex = 0;
    m_lfo.reset();
}

void PitchModulator::setModulation(float rate, float depth)
{
    m_rate = juce::jlimit(0.1f, 10.0f, rate);
    m_depth = juce::jlimit(0.0f, 100.0f, depth);
    
    m_lfo.setFrequency(m_rate);
}

float PitchModulator::processSample(float inputSample)
{
    // Write input to delay buffer
    m_delayBuffer[m_writeIndex] = inputSample;
    
    // Get LFO modulation
    float lfoValue = m_lfo.getNextSample();
    
    // Convert to pitch shift in cents
    float pitchShiftCents = lfoValue * m_depth;
    
    // Convert to delay time ratio
    float delayRatio = centsToDelayRatio(pitchShiftCents);
    
    // Calculate modulated delay time
    float baseDelay = static_cast<float>(m_maxDelayInSamples) * 0.5f; // Use half buffer as base
    float modulatedDelay = baseDelay * delayRatio;
    
    // Read from delay buffer with interpolation
    float output = readDelayBuffer(modulatedDelay);
    
    // Advance write index
    m_writeIndex = (m_writeIndex + 1) % m_maxDelayInSamples;
    
    return output;
}

float PitchModulator::centsToDelayRatio(float cents) const
{
    // Convert cents to frequency ratio, then invert for delay ratio
    float freqRatio = std::pow(2.0f, cents / 1200.0f);
    return 1.0f / freqRatio;
}

float PitchModulator::readDelayBuffer(float delayInSamples) const
{
    if (m_delayBuffer.empty())
        return 0.0f;
    
    // Calculate read position
    float readPos = static_cast<float>(m_writeIndex) - delayInSamples;
    
    // Wrap around buffer
    while (readPos < 0.0f)
        readPos += static_cast<float>(m_maxDelayInSamples);
    while (readPos >= static_cast<float>(m_maxDelayInSamples))
        readPos -= static_cast<float>(m_maxDelayInSamples);
    
    // Linear interpolation
    int index1 = static_cast<int>(readPos);
    int index2 = (index1 + 1) % m_maxDelayInSamples;
    float fraction = readPos - static_cast<float>(index1);
    
    return m_delayBuffer[index1] * (1.0f - fraction) + m_delayBuffer[index2] * fraction;
}