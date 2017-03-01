%% Load files
load('keys1.mat');
load('acceleration1.mat');

%% Filter the signal from acceleration
key_time = keys(:,1);
press = keys(:,2);

Fs = 50;                    % sample rate in Hz
N = length(acceleration);                     % number of signal samples
rng default;

% Design a 70th order lowpass FIR filter with cutoff frequency of 75 Hz.
Fnorm = 23/(Fs/2);           % Normalized frequency
df = designfilt('highpassfir','FilterOrder',30,'CutoffFrequency',Fnorm);
mag = acceleration(:,5);
acc_time = acceleration(:,1)/1000;

diff = abs(acc_time(1) - key_time(1));
if (acc_time(1) > key_time(1))
    acc_time = acc_time + diff;
else 
    key_time = key_time + diff;
end

acc_time = acc_time - acc_time(1);
key_time = key_time - key_time(1);

y = filter(df, mag);

mu = mean(y)
sigma = std(y)

[D, PD] = allfitdist(y, 'PDF');

fprintf('%10s\t'  , D(1).ParamNames{:}); fprintf('\n');
fprintf('%10.2f\t', D(4).Params       ); fprintf('\n');
%subplot(2,1,1);
%plot(key_time, press, 'r')
%grid on
%ylim([1 3])
%hold on

%subplot(2,1,2);
%plot(acc_time, press);
%ylim([1 3])
%grid on
