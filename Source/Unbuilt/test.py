import numpy as np
from scipy import stats
alpha = 0.05 # 有意水準
sigma = 8 # 標準偏差
n = 100 # 標本数
df = n-1 # 自由度
X = 150 # 帰無仮説における平均値
x = 148 # 標本平均
se = sigma / np.sqrt(n) # 標準誤差
interval = stats.t.interval(1-alpha, df = df, loc = X, scale = se)
print("棄却域(採択域の境界): " + str(interval))
if x < interval[0] or x > interval[1]:
    print("帰無仮説は棄却され、対立仮説が採用される")
    print("平均ログ量は150万件ではない")
else:
    print("帰無仮説は採択され、対立仮説は採用されない")
    print("平均ログ量は150万件ではないとは言えない")