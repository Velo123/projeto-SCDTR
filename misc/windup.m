time=windup_velo.time;
antiwindup=windup_velo.windup_state;
ref=windup_velo.lux_ref;
meas=windup_velo.lux_meas;
pwm=windup_velo.duty_cycle;

s_aw_on=9744;
s_aw_off=12469;

e_aw_on=10544;
e_aw_off=13269;


aw_time=time(s_aw_on:e_aw_on);
aw_ref=ref(s_aw_on:e_aw_on);
aw_meas=meas(s_aw_on:e_aw_on);
aw_pwm=pwm(s_aw_on:e_aw_on);

naw_time=time(s_aw_off:e_aw_off);
naw_ref=ref(s_aw_off:e_aw_off);
naw_meas=meas(s_aw_off:e_aw_off);
naw_pwm=pwm(s_aw_off:e_aw_off);


%%
figure

colors = lines(5);

%% --- WITH ANTI-WINDUP ---
subplot(2,2,1)

plot(aw_time,aw_ref,'--k','LineWidth',1.5); hold on
plot(aw_time,aw_meas,'Color',colors(1,:),'LineWidth',1.5)

ylabel('Illuminance (lux)')
title('With Anti-Windup')

grid on
box on

legend('Reference','Measurement','Location','best')

subplot(2,2,3)

plot(aw_time,aw_pwm,'Color',colors(4,:),'LineWidth',1.5)
ylabel('PWM duty')
xlabel('Time (s)')

ylim([0 1])
grid on
box on

%% --- WITHOUT ANTI-WINDUP ---
subplot(2,2,2)

plot(naw_time,naw_ref,'--k','LineWidth',1.5); hold on
plot(naw_time,naw_meas,'Color',colors(1,:),'LineWidth',1.5)

ylabel('Illuminance (lux)')
title('Without Anti-Windup')

grid on
box on

legend('Reference','Measurement','Location','best')

subplot(2,2,4)

plot(naw_time,naw_pwm,'Color',colors(4,:),'LineWidth',1.5)

ylabel('PWM duty')
xlabel('Time (s)')

ylim([0 1])
grid on
box on

set(gcf,'Color','white')
set(gca,'FontSize',12)