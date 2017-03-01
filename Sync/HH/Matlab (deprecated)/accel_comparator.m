% grab files
crystal = csvread('Data/Crystal_accel_COM6.csv');
dco = csvread('Data/DCO_accel_COM9.csv');

% crystal file likely to be longer than DCO file, due to different clock
% frequencies (Crystal: 4MHz, DCO: 2MHz)
diff = abs(length(crystal) - length(dco));

% compensate for DCO
if(length(crystal) > length(dco))
 %   crystal = crystal(diff+1:end,:);
   crystal = crystal(1:end-diff,:);
else
 %   dco = dco(diff+1:end,:);
    dco = dco(1:end-diff,:);
end

% Design a 70th order lowpass FIR filter with cutoff frequency of 75 Hz.
Fs = 100;                    % sample rate ~100 Hz
Fnorm = 45/(Fs/2);           % Normalized frequency
N = length(crystal);
df = designfilt('highpassfir','FilterOrder',10,'CutoffFrequency',Fnorm);

% Filter and set up x and y axes for crystal and DCO
cmag = crystal(:,2).^2 + crystal(:,3).^2 + crystal(:,4).^2;
y1 = filtfilt(df, cmag);
cmag = sqrt(cmag);
ctime = crystal(:,1);
ctime = ctime - ctime(1);

dmag = dco(:,2).^2 + dco(:,3).^2 + dco(:,4).^2;
dmag = sqrt(dmag);
y2 = filtfilt(df, dmag);
dtime = dco(:,1);
dtime = dtime - dtime(1);

% Plot all the info so far.
subplot(2,1,1);
plot(ctime, cmag);
title('Crystal Clock');
xlabel('Time (microsec)');
ylabel('Magnitude');
vline(37380000,'-','S1')
grid on;

subplot(2,1,2);
plot(dtime, dmag, 'r');
title('DCO Clock');
xlabel('Time (microsec)');
ylabel('Magnitude');
vline(37380000,'-','S2')
grid on;


% X-Corr sandbox
[acor, lag] = xcorr(dmag, cmag);
[~,I] = max(abs(acor));
lagDiff = lag(I)
timeDiff = lagDiff/Fs

s1al = cmag(-lagDiff+1:end);
t1al = (0:length(s1al)-1)/Fs;
% 
% subplot(2,1,1)
% plot(t1al, s1al)
% title('s_1, aligned')
% 
% subplot(2,1,2)
% plot(dtime,dmag)
% title('s_2')
% xlabel('Time (s)')
% 

