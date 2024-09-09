num_samples = 50000;
% using smaller freqs
LO_freq = 1e6;
BB_freq = 100e3;
sample_rate = 25e6;

% UPMIXING: LO freq, BB
LO_arr = generate_iq_at_freq(num_samples,LO_freq,1,sample_rate);
BB_arr = generate_iq_at_freq(num_samples,BB_freq,1,sample_rate);
upmixed = LO_arr.*BB_arr;
% DOWNMIXING
IF_arr = upmixed;
downmixed = IF_arr.*conj(LO_arr); % conjugate used in downmixing

plot(real(fft(upmixed)))
hold on
plot(real(fft(downmixed)))



function complex_data = generate_iq_at_freq(num_samples, freq, amplitude, sample_rate)
    t = linspace(0, num_samples / sample_rate, num_samples);
    I = amplitude * cos(2 * pi * freq * t);
    Q = amplitude * sin(2 * pi * freq * t);
    complex_data = I + 1i * Q;
end