% Read the Output bin file
filename = '/home/murray/sdr-config/outputs/pulsed_test.bin';
num_samples = 5e6;
% using smaller freqs
LO_freq = 1e6;
BB_freq = 0;
refl_freq = 0;
sample_rate = 25e6;
c = physconst('LightSpeed');

% Pulse params
pulse_length = 2500;
pulse_separation = 25000;
prf = sample_rate/pulse_separation;
pulse_length_secs = pulse_length/sample_rate;
pulse_separation_secs = pulse_separation/sample_rate;
range_res = (c*pulse_length_secs)/(2); % c/2B
range_max = c*pulse_separation_secs/2; % c*PRI/2
disp(['Range resolution: ', num2str(range_res)]);
disp(['Max range: ', num2str(range_max)]);

fileID = fopen(filename, 'r');
data = fread(fileID, Inf, 'double');
fclose(fileID);
realPart = data(1:2:end);  % Odd indices: real part
imagPart = data(2:2:end);  % Even indices: imaginary part
downmixed = realPart + 1i * imagPart;
start_idx = 100000;
downmixed = downmixed(start_idx:end);
num_samples = num_samples-start_idx;

% Read the template pulse
filename = '/home/murray/sdr-config/sweep.csv';
data = readmatrix(filename);
realPart = data(:, 1);  % First column is the real part
imagPart = data(:, 2);  % Second column is the imaginary part
BB_arr = realPart + 1i * imagPart;
rx_lo_arr = generate_iq_at_freq(size(BB_arr),800e6,1,25e6);
tx_lo_arr = generate_iq_at_freq(size(BB_arr),801e6,1,25e6);
upmixed_pulse = tx_lo_arr.*BB_arr;
BB_arr = upmixed_pulse.*conj(rx_lo_arr);

% 1. Lower signal to baseband
% 2. For the receiving part of the n'th PRI, store the data in the n'th row of a matrix.
% 3. For each row, Cross-correlate it with the transmitted signal
% 4. DFT each column
% 5. Plot the doppler map





% Timestamping for first pulse (USRP will have delay between Rx and Tx startups)
template_pulse = BB_arr(1:pulse_length);
corr_end = pulse_length * 2;
corr = xcorr(downmixed(1:corr_end), template_pulse);
% ignore first half
corr = corr(pulse_length:end);
first_pulse_index = find(corr == max(corr), 1) - pulse_length;
disp(['First pulse is at index: ', num2str(first_pulse_index)]);
% NOT WORKING :(
% manually setting start idx



% extract segment
% TODO: upsample here to increase range resolution
%pulse_end = round(first_pulse_index+length(template_pulse));
%segment = downmixed(pulse_end:pulse_end+pulse_separation-pulse_length);
% now correlate template pulse with the segment to get the time delay
%correlation_seg = xcorr(segment, template_pulse);
%correlation_seg = correlation_seg(pulse_separation-pulse_length:end);
%peak_index = find(correlation_seg == max(correlation_seg), 1); %- (length(template_pulse)-1);
%disp(['Echo pulse is at index: ', num2str(peak_index)]);
%td = peak_index/sample_rate;
%d = td*c; % /2 ???
%disp(['Distance of target: ', num2str(d)]);



% Calculate range-Doppler response using FFT
received_signal = downmixed(first_pulse_index:end);
target_signal = BB_arr(1:pulse_length);
% Store data in a matrix
num_pulses = 17;
received_data = zeros(pulse_separation, num_pulses);
segment_start = pulse_length;
for n = 1:num_pulses
    if (segment_start+pulse_separation>size(received_signal))
        temp = received_signal(segment_start:size(received_signal)); 
        %received_data(:, n) = [temp;zeros(pulse_separation-size(temp,1),1)];       
    else 
        received_data(:, n) = received_signal(segment_start:segment_start+pulse_separation-1);
    end
    segment_start = segment_start + pulse_separation;
    %disp(num2str(segment_start));
end

% Cross-correlate each row with the transmitted signal
correlation_matrix = zeros(pulse_separation-500, num_pulses);
for n = 1:num_pulses
    corr_data = xcorr(received_data(:, n), target_signal);
    correlation_matrix(:,n) = corr_data(pulse_separation:end-500);
end
% Summing to increase SNR across a number of pulses
%summed_pulse = zeros(1, pulse_separation-pulse_length);
%for n = 1:num_pulses
%    summed_pulse = summed_pulse+correlation_matrix(n,:);
%end

num_bins = ceil(range_max/range_res);
x_axis = (0:num_bins)*range_res;
y_axis = (0:num_pulses);

figure;
imagesc(x_axis,y_axis,20*log10(abs(fftshift(fft(correlation_matrix'),1))));
ylabel('Doppler (Hz)');
xlabel('Range (m)');
title('Range-Doppler Map');
axis xy;

% Create figure
figure;

% First subplot
subplot(3, 1, 1)
p1 = real(received_data(:,1));
plot(p1);
title('First Plot');

% Second subplot
subplot(3, 1, 2);
p2 = real(template_pulse);
plot(p2);
title('Second Plot');

% Third subplot
subplot(3, 1, 3);
p3 = real(xcorr(p1,p2));
plot(p3(25000:end));
title('Third Plot');

%imagesc(20*log10(abs(fftshift(fft(fft2(received_data))))))


function complex_data = generate_iq_at_freq(num_samples, freq, amplitude, sample_rate)
    t = linspace(0, num_samples / sample_rate, num_samples);
    I = amplitude * cos(2 * pi * freq * t);
    Q = amplitude * sin(2 * pi * freq * t);
    complex_data = I + 1i * Q;
end