local secs = NowEpochSeconds()
local t = {}
for j = 1, 10000000 do
    t[j] = j
end
print(NowEpochSeconds() - secs)

require('res.test2')
