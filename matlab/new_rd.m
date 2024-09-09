% 1. Lower signal to baseband
% 2. For the receiving part of the n'th PRI, store the data in the n'th row of a matrix.
% 3. For each row, Cross-correlate it with the transmitted signal
% 4. DFT each column
% 5. Plot the doppler map



num_samples = 5e6;
% using smaller freqs
LO_freq = 1e6;
BB_freq = 100e3;
refl_freq = 100e3;
sample_rate = 25e6;
c = physconst('LightSpeed');

% Pulse params
pulse_length = 50;
pulse_separation = 10000;
prf = sample_rate/pulse_separation;
range_res = (c*pulse_length)/(2*sample_rate);
disp(['Range resolution: ', num2str(range_res)]);

% UPMIXING: LO freq, BB
LO_arr = generate_iq_at_freq(num_samples,LO_freq,1,sample_rate);
BB_arr = generate_iq_at_freq(num_samples,BB_freq,1,sample_rate);
refl_arr = generate_iq_at_freq(num_samples,refl_freq,1,sample_rate);
num_pulses = 0;
for i = pulse_length:pulse_separation:length(BB_arr)
    BB_arr(i:i+pulse_separation-pulse_length) = 0;
    num_pulses = num_pulses + 1;
end
% truncate back down to size
BB_arr = BB_arr(1:num_samples);
upmixed = LO_arr.*BB_arr;
upmixed_refl = LO_arr.*refl_arr;
% Add noise
noise_variance = 0.01;
scale_factor = 0.05;
noise_scale_factor = 0.5;
noise_i = scale_factor * noise_scale_factor * randn(1, num_samples) * sqrt(noise_variance);
noise_q = scale_factor * noise_scale_factor * randn(1, num_samples) * sqrt(noise_variance);
upmixed = upmixed + noise_i + 1i * noise_q;
% insert echo
d = 100000;
td = d/c;
idx = round(td*sample_rate);
for i = pulse_length:pulse_separation:length(upmixed)
    upmixed(i+idx:i+idx+pulse_length) = scale_factor*upmixed_refl(1:pulse_length+1);
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



% Calculate range-Doppler response using FFT
received_signal = downmixed;
target_signal = [BB_arr(1:pulse_length), zeros(1,pulse_separation-2*pulse_length)];
% Store data in a matrix
%received_data = zeros(num_pulses, pulse_separation-pulse_length); % TODO: change to pulse_length,num_pulses
%received_data = zeros(num_pulses, pulse_separation);
received_data = zeros(pulse_separation-pulse_length, num_pulses);
segment_start = pulse_length;
for n = 1:num_pulses
    %received_data(n, :) = received_signal(segment_start:segment_start+pulse_separation-1-pulse_length);
    %received_data(n, :) = received_signal(1:pulse_separation);
    received_data(:, n) = received_signal(segment_start:segment_start+pulse_separation-1-pulse_length);
    segment_start = segment_start + pulse_separation;
end



pulse_width_secs = pulse_length/sample_rate;
waveform = phased.RectangularWaveform('PulseWidth',pulse_width_secs,...
    'OutputFormat','Pulses','PRF',prf,'NumPulses',1,'SampleRate',sample_rate,'FrequencyOffset',BB_freq);
rangedoppler = phased.RangeDopplerResponse(...
    'RangeMethod','Matched filter',...
    'PropagationSpeed',c,...
    'DopplerOutput','Speed','OperatingFrequency',LO_freq);
plotResponse(rangedoppler,received_data,getMatchedFilter(waveform))
%plotResponse(rangedoppler,received_data)


imagesc(abs(fftshift(fft2(received_data'))))


function complex_data = generate_iq_at_freq(num_samples, freq, amplitude, sample_rate)
    t = linspace(0, num_samples / sample_rate, num_samples);
    I = amplitude * cos(2 * pi * freq * t);
    Q = amplitude * sin(2 * pi * freq * t);
    complex_data = I + 1i * Q;
end