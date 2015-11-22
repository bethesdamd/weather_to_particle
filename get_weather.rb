# Get temperature from openweather.org and send it to Particle board
require 'json'
require 'net/http'
require 'pp'
require 'yaml'

config = YAML.load_file('PASSWORDS.NO.COMMIT')
PARTICLE_DEVICE_ID = 'aardvark_crazy'
FORECAST_CITY='4348599'
url = URI.parse('http://api.openweathermap.org/data/2.5/forecast?id=' + FORECAST_CITY + '&APPID=' + config['OPEN_WEATHER_APPID'])
req = Net::HTTP::Get.new(url.to_s)
res = Net::HTTP.start(url.host, 80) { |http| http.request(req) }

j = JSON.parse(res.body)
k = j['list'][0]['main']['temp']
# pp j 
fahr = ((k.to_f - 273.15) * 1.8 + 32.0).to_i.to_s
puts fahr

particle_uri = URI("https://api.particle.io/v1/devices/" + PARTICLE_DEVICE_ID + "/remoteTemp")

command = "curl https://api.particle.io/v1/devices/#{PARTICLE_DEVICE_ID}/remoteTemp -d arg=#{fahr} -d access_token=#{config['PARTICLE_ACCESS_TOKEN']}"

puts "command: #{command}"
ret = system(command)
# puts "response from curl: #{ret}"

# curl https://api.particle.io/v1/devices/aardvark_crazy/reoteTemp -d arg="30" -d access_token=a8a66512b6b46995c6997361d82c0115aff8c49e

=begin
puts particle_uri
https = Net::HTTP.new(particle_uri.host, 80)
https.use_ssl = false 
req = Net::HTTP::Post.new(particle_uri.path)
req["arg"] = "50"
req["access_token"] = config['PARTICLE_ACCES_TOKEN']
response = https.request(req)
puts response
=end


