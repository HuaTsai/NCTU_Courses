cvx_begin
   variable x
   minimize(log(exp(x)+exp(-x)))
cvx_end

disp(['Optimal value: ', sprintf('%f', cvx_optval)]);
disp(['Optimal point: ', sprintf('%f ', x)]);
