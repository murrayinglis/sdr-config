% NOTE: my USRP has the spike which settles after about 75000 samples.


% Read the Output bin file
filename = '../outputs/pulsed_test.bin';
fileID = fopen(filename, 'r');
data = fread(fileID, Inf, 'double');
fclose(fileID);
realPart = data(1:2:end)';  % Odd indices: real part
imagPart = data(2:2:end)';  % Even indices: imaginary part
downmixed = realPart + 1i * imagPart;
downmixed = downmixed(75001:end);
% Filter 
fs = 25e6;                 % Sampling frequency in Hz
cutoffFreq = 0.1;          % Cutoff frequency 
d = designfilt('highpassiir', 'FilterOrder', 2, ...
               'HalfPowerFrequency', cutoffFreq, ...
               'SampleRate', fs);
downmixed = filtfilt(d, downmixed);

% Read the template pulse
filename = '../sweep.csv';
data = readmatrix(filename);
realPart = data(:, 1)';  % First column is the real part
imagPart = data(:, 2)';  % Second column is the imaginary part
BB_arr = realPart + 1i * imagPart;


% 1. Lower signal to baseband
% 2. For the receiving part of the n'th PRI, store the data in the n'th row of a matrix.
% 3. For each row, Cross-correlate it with the transmitted signal
% 4. DFT each column
% 5. Plot the doppler map


num_samples = 500000;
% using smaller freqs
LO_freq = 800e6;
refl_freq = 0;
sample_rate = 25e6;
c = physconst('LightSpeed');

% Pulse params
pulse_length = 2500;
pulse_separation = 25000;
prf = sample_rate/pulse_separation;
num_pulses = 17;
range_res = (c*pulse_length)/(2*sample_rate);
disp(['Range resolution: ', num2str(range_res)]);



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
received_data = zeros(pulse_separation-pulse_length, num_pulses);
segment_start = pulse_length;
for n = 1:num_pulses
    received_data(:, n) = received_signal(segment_start:segment_start+pulse_separation-1-pulse_length);
    segment_start = segment_start + pulse_separation;
end

% Cross-correlate each row with the transmitted signal
correlation_matrix = zeros(pulse_separation-pulse_length, num_pulses);
for n = 1:num_pulses
    corr_data = xcorr(received_data(:, n), target_signal);
    correlation_matrix(:,n) = corr_data(pulse_separation-pulse_length:end);
end
% Summing to increase SNR across a number of pulses
%summed_pulse = zeros(1, pulse_separation-pulse_length);
%for n = 1:num_pulses
%    summed_pulse = summed_pulse+correlation_matrix(n,:);
%end

range_bin_size = c/(2*sample_rate);
v_bin_size = (c*prf)/(2*num_pulses);

fft_data = fft2(correlation_matrix); % Range map
response = fftshift(fft(fft_data),2); % Range-doppler map
range_grid = (0:num_pulses-1)/sample_rate * c/2; % Range grid
doppler_grid = (-prf/2:prf/2-1)/num_pulses; % Doppler grid


imagesc(doppler_grid, range_grid, 20*log10(abs(response)));
xlabel('Doppler (Hz)');
ylabel('Range (m)');
title('Range-Doppler Map');
colorbar;


%imagesc(20*log10(abs(fftshift(fft(fft2(received_data))))))


function complex_data = generate_iq_at_freq(num_samples, freq, amplitude, sample_rate)
    t = linspace(0, num_samples / sample_rate, num_samples);
    I = amplitude * cos(2 * pi * freq * t);
    Q = amplitude * sin(2 * pi * freq * t);
    complex_data = I + 1i * Q;
end