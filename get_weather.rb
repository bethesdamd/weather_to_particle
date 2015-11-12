# Get temperature from openweather.org and send it to Particle board
require 'json'
require 'net/http'
require 'pp'
require 'yaml'

config = YAML.load_file('PASSWORDS.NO.COMMIT')

FORECAST_CITY='4348599'
url = URI.parse('http://api.openweathermap.org/data/2.5/forecast?id=' + FORECAST_CITY + '&APPID=' + config['OPEN_WEATHER_APPID'])
req = Net::HTTP::Get.new(url.to_s)
res = Net::HTTP.start(url.host, 80) { |http| http.request(req) }

j = JSON.parse(res.body)
k = j['list'][0]['main']['temp']
# pp j 
fahr = (k.to_f - 273.15)*1.8 + 32.0
puts fahr.to_i

