zones = {}

protocols = {}
protocols['1'] = 'icmp'
protocols['6'] = 'tcp'
protocols['17'] = 'udp'

-- ----------------------------------------------------------------------
--
-- update
--
--
-- timestamp       2014-08-09T00:15:36.574
-- linenum 1
-- syslog-src      172.17.2.1
-- action  2014-08-09T00:15:36.574
-- jlog    table: 0x1ab8170 (see below for entries)

-- logical-system-name     BOL-BDC-LS01
-- source-zone-name        BOL-WAN
-- destination-zone-name   BOL-LEG-Voters_Server_Farm

-- policy-name     catch-all

-- source-address  10.145.227.41
-- nat-source-address      10.145.227.41
-- destination-address     10.20.15.5
-- nat-destination-address 10.20.15.5

-- protocol-id     6

-- source-port     1575
-- nat-source-port 1575
-- destination-port        8192
-- nat-destination-port    8192
-- service-name    None

-- packet-incoming-interface       ge-2/0/1.0
-- roles   N/A
-- username        N/A
-- src-nat-rule-name       None
-- dst-nat-rule-name       None
-- session-id-32   20425157

-- ----------------------------------------------------------------------

function update(t)

   if (tdiff ~= nil)
   then
      if (t['timestamp'] < tdiff)
      then
	 return 0
      end
   end
   
   local jlog = t.jlog

--   for k,v in pairs(jlog) do
--      print(k, v)
--   end

   local zdn = jlog['destination-zone-name'] 
   if zones[zdn] == nil then
      print(string.format("Adding zone %s", zdn))
      zones[zdn] = {}
   end
   local zd = zones[zdn]

   local adn = jlog['destination-address']
   if zd[adn] == nil then
      print(string.format("Adding zone %s address %s", zdn, adn))
      zd[adn] = {}
   end
   local da = zd[adn]

   local pdn = string.format("%s/%s", protocols[jlog['protocol-id']], jlog['destination-port'])
   if jlog['protocol-id'] == '1' then
      pdn = "icmp/-"
   end

   if da[pdn] == nil then
      print(string.format("Adding zone %s address %s port %s", zdn, adn, pdn))
      da[pdn] = {}
   end
   local dp = da[pdn]

-- Source zone

   local zsn = jlog['source-zone-name']
   if dp[zsn] == nil then
      print(string.format("Adding zone %s address %s port %s zone %s", zdn, adn, pdn, zsn))
      dp[zsn] = {}
   end
   local zs = dp[zsn]


   local asn = jlog['source-address']
   if zs[asn] == nil then
      print(string.format("Adding zone %s address %s port %s zone %s address %s", zdn, adn, pdn, zsn, asn))
      zs[asn] = 0
   end
   zs[asn] = zs[asn] + 1

   return 1
end

-- ----------------------------------------------------------------------
--
-- dump
--
-- ----------------------------------------------------------------------

function dump()
   for dzn,dzone in pairs(zones) do
      for dan, daddress in pairs(dzone) do
	 for pn, sz in pairs(daddress) do
	    for szn, saddress in pairs(sz) do
	       for san, count in pairs(saddress) do
		  print(string.format("%s\t%s\t%s\t%s\t%s\t%d", szn, san, dzn, dan, pn, count))
	       end
	    end
	 end
      end
   end
end