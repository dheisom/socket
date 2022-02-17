local net = require 'net'

s = net.tcp(5000)
net.listen(s)
c, err = net.accept(s)
if err ~= nil then
  print(err)
  goto close
end
net.write(c, "envia algo ai\n")
io.read()
print(net.read(c, 1<<16))
net.write(c, "Toma um pouco de dados\n")

::close::
net.close(c)
net.close(s)
