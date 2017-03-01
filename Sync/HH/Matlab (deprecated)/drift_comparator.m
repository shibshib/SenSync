crystal = csvread('Data/Crystal.csv');
dco = csvread('Data/DCO.csv');

drift = abs(crystal - dco);

plot(drift)
title('Drift between the Crystal oscillator and the DCO onboard MSP430');
xlabel('Samples')
ylabel('Time (micros)')
legend('Crystal vs DCO');
grid on


