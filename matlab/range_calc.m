num_samples = 500000;
% using smaller freqs
LO_freq = 1e6;
BB_freq = 100e3;
sample_rate = 25e6;
c = 3e8;

% Pulse params
pulse_length = 5000;
pulse_separation = 40000;
prf = sample_rate/pulse_separation;

% UPMIXING: LO freq, BB
LO_arr = generate_iq_at_freq(num_samples,LO_freq,1,sample_rate);
BB_arr = generate_iq_at_freq(num_samples,BB_freq,1,sample_rate);
num_pulses = 0;
for i = pulse_length:pulse_separation:length(BB_arr)
    BB_arr(i:i+pulse_separation-pulse_length) = 0;
    num_pulses = num_pulses + 1;
end
% truncate back down to size
BB_arr = BB_arr(1:num_samples);
upmixed = LO_arr.*BB_arr;
% Add noise
noise_variance = 0.01;
scale_factor = 0.005;
noise_i = scale_factor * randn(1, num_samples) * sqrt(noise_variance);
noise_q = scale_factor * randn(1, num_samples) * sqrt(noise_variance);
upmixed = upmixed + noise_i + 1i * noise_q;
% insert echo
d = 1000;
td = d/c;
idx = round(td*sample_rate);
for i = pulse_length:pulse_separation:length(upmixed)
    upmixed(i+idx:i+idx+pulse_length) = scale_factor*upmixed(1:pulse_length+1);
end

% DOWNMIXING
IF_arr = upmixed;
downmixed = IF_arr.*conj(LO_arr); % conjugate used in downmixing


% Timestamping for first pulse (USRP will have delay between Rx and Tx startups)
template_pulse = BB_arr(1:pulse_length);
corr_end = pulse_length * 2;
corr = xcorr(downmixed(1:corr_end), template_pulse);
% ignore first half
corr = corr(pulse_length:end);
first_pulse_index = find(corr == max(corr), 1) - pulse_length;
disp(['First pulse is at index: ', num2str(first_pulse_index)]);
% extract segment
% TODO: upsample here to increase range resolution
pulse_end = round(first_pulse_index+length(template_pulse));
segment = downmixed(pulse_end:pulse_end+pulse_separation-pulse_length);
% now correlate template pulse with the segment to get the time delay
correlation_seg = xcorr(segment, template_pulse);
correlation_seg = correlation_seg(pulse_separation-pulse_length:end);
peak_index = find(correlation_seg == max(correlation_seg), 1); %- (length(template_pulse)-1);
disp(['Echo pulse is at index: ', num2str(peak_index)]);
td = peak_index/sample_rate;
d = td*c; % /2 ???
disp(['Distance of target: ', num2str(d)]);

% Plotting
subplot(1, 2, 1); % 1 rows, 2 columns, 1st subplot
plot(real(fft(upmixed)));
hold on
plot(real(fft(downmixed)));
subplot(1, 2, 2); % 1 rows, 2 columns, 2nd subplot
plot(real(downmixed))
hold off

function complex_data = generate_iq_at_freq(num_samples, freq, amplitude, sample_rate)
    t = linspace(0, num_samples / sample_rate, num_samples);
    I = amplitude * cos(2 * pi * freq * t);
    Q = amplitude * sin(2 * pi * freq * t);
    complex_data = I + 1i * Q;
end