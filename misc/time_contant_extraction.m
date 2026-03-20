R_FIXED=10000;
b=6.1;
m=-0.8;

% time02=step_duty_0_2.timestamp_ms;
% led02=step_duty_0_2.adc_value;
% adc02=step_duty_0_2.voltage_v;
% v_out02=(adc02* 3.3) / 4095.0;
% r02=(R_FIXED * (3.3 - v_out02)) ./ v_out02;
% lux02=10.^((log10(r02)-b)/m);

time04=step_duty_0_4.timestamp_ms;
led04=step_duty_0_4.adc_value;
adc04=step_duty_0_4.voltage_v;
v_out04=(adc04* 3.3) / 4095.0;
r04=(R_FIXED * (3.3 - v_out04)) ./ v_out04;
lux04=10.^((log10(r04)-b)/m);

time06=step_duty_0_6.timestamp_ms;
led06=step_duty_0_6.adc_value;
adc06=step_duty_0_6.voltage_v;
v_out06=(adc06* 3.3) / 4095.0;
r06=(R_FIXED * (3.3 - v_out06)) ./ v_out06;
lux06=10.^((log10(r06)-b)/m);

time08=step_duty_0_8.timestamp_ms;
led08=step_duty_0_8.adc_value;
adc08=step_duty_0_8.voltage_v;
v_out08=(adc08* 3.3) / 4095.0;
r08=(R_FIXED * (3.3 - v_out08)) ./ v_out08;
lux08=10.^((log10(r08)-b)/m);

time1=step_duty_1.timestamp_ms;
led1=step_duty_1.adc_value;
adc1=step_duty_1.voltage_v;
v_out1=(adc1* 3.3) / 4095;
r1=(R_FIXED * (3.3 - v_out1)) ./ v_out1;
lux1=10.^((log10(r1)-b)/m);
% 
% % lux02=movmean(lux02,10);
lux04=movmean(lux04,10);
lux06=movmean(lux06,10);
lux08=movmean(lux08,10);
lux1=movmean(lux1,10);
% 
% lux04=lux04(4600:end);
% time04=time04(4600:end);
% lux08=lux08(4600:end);
% time08=time08(4600:end);
% lux06=lux06(4600:end);
% time06=time06(4600:end);
% 

% x02=lux02(1)-((lux02(1)-lux02(end))*0.63);
% x04=lux04(1)-((lux04(1)-lux04(end))*0.63);
% x06=lux06(1)-((lux06(1)-lux06(end))*0.63);
% x08=lux08(1)-((lux08(1)-lux08(end))*0.63);
% x1=lux1(1)-((lux1(1)-lux1(end))*0.63);


% x02=lux02(end)*0.63;
x04=lux04(end)*0.63;
x06=lux06(end)*0.63;
x08=lux08(end)*0.63;
x1=lux1(end)*0.63;


% [~, idx02] = min(abs(lux02 - x02));
[~, idx04] = min(abs(lux04 - x04));
[~, idx06] = min(abs(lux06 - x06));
[~, idx08] = min(abs(lux08 - x08));
[~, idx1] = min(abs(lux1 - x1));
% 
% Convert timestamp from milliseconds to seconds
% time02 = time02 / 1000;
time04 = time04 / 1000;
time06 = time06 / 1000;
time08 = time08 / 1000;
time1 = time1 / 1000;


% x002 = time02(idx02);
x004 = time04(idx04);
x006 = time06(idx06);
x008 = time08(idx08);
x01 = time1(idx1);

figure
    colors = lines(5);

% Plot filtered signals
% plot(time02,lux02,'Color',colors(1,:),'LineWidth',1.5); hold on
plot(time04,lux04,'Color',colors(2,:),'LineWidth',1.5); hold on
plot(time06,lux06,'Color',colors(3,:),'LineWidth',1.5)
plot(time08,lux08,'Color',colors(4,:),'LineWidth',1.5)
plot(time1 ,lux1,'Color',colors(5,:),'LineWidth',1.5) 


xlim([9.5 12])
% ylim([0 1.2])
yl = ylim;


step_time = 10;

plot([step_time step_time], yl,'k--','LineWidth',1.5)

text(step_time, yl(2)*0.95, ' Step input', ...
    'FontSize',12,'FontWeight','bold','Color','k', ...
    'VerticalAlignment','top')

%63% levels
% y02 = x02;
y04 = x04;
y06 = x06;
y08 = x08;
y1  = x1;

% vertical tau lines
% plot([x002 x002], yl,'--','Color',colors(1,:),'LineWidth',1)
plot([x004 x004], yl,'--','Color',colors(2,:),'LineWidth',1)
plot([x006 x006], yl,'--','Color',colors(3,:),'LineWidth',1)
plot([x008 x008], yl,'--','Color',colors(4,:),'LineWidth',1)
plot([x01  x01 ], yl,'--','Color',colors(5,:),'LineWidth',1)

% horizontal 63% lines
% plot([min(time02) x002],[y02 y02],':','Color',colors(1,:),'LineWidth',1)
plot([min(time04) x004],[y04 y04],':','Color',colors(2,:),'LineWidth',1)
plot([min(time06) x006],[y06 y06],':','Color',colors(3,:),'LineWidth',1)
plot([min(time08) x008],[y08 y08],':','Color',colors(4,:),'LineWidth',1)
plot([min(time1)  x01 ],[y1  y1 ],':','Color',colors(5,:),'LineWidth',1)

% tau annotations
% text(x002,y02,sprintf('  \\tau=%.2fs',x002),'Color',colors(1,:),'FontSize',14,'FontWeight','bold')
text(x004,y04,sprintf('  \\tau=%.2fs',x004),'Color',colors(2,:),'FontSize',14,'FontWeight','bold')
text(x006,y06,sprintf('  \\tau=%.2fs',x006),'Color',colors(3,:),'FontSize',14,'FontWeight','bold')
text(x008,y08,sprintf('  \\tau=%.2fs',x008),'Color',colors(4,:),'FontSize',14,'FontWeight','bold')
text(x01 ,y1 ,sprintf('  \\tau=%.2fs',x01 ),'Color',colors(5,:),'FontSize',14,'FontWeight','bold')

grid on
box on

title('Step Response of Luminaire System','FontSize',14)
ylabel('Illuminance (lux)')
xlabel('Time (s)')

% legend({'Duty 0.2','Duty 0.4','Duty 0.6','Duty 0.8','Duty 1.0',...
%         '\tau location','63% level'}, ...
%         'Location','southeast')


legend({'Duty 0.4','Duty 0.6','Duty 0.8','Duty 1.0',...
        '\tau location','63% level'}, ...
        'Location','southeast')

set(gca,'FontSize',12)
set(gcf,'Color','white')
