-- ----------------------------------------------------------------------
--
-- 2023.04.05
--
-- Simple script called by dvdrip.
--
-- There are two functions, currently check_parsms and process_titles
--
-- ----------------------------------------------------------------------

function check_params(params)
   print(params.device)
   print(params.file)
   print(params.index)

   if (params.separator == "") then
      print("No searator specified")
      params.separator = "."
   end
   if (params.path == "") then
      params.path = "."
   end
   return false
end

function process_titles(dvdrip, titles)
   local index = dvdrip.index
   for title = 0, titles:size() - 1, 1 do
      if (titles[title].duration > 1000) then
	 print(titles[title].duration)
	 local media_file = string.format("%s/%s%s%02d.mpg", dvdrip.path, dvdrip.file, dvdrip.separator, index)
	 dvd.dvdrip_read_title(dvdrip, titles[title], media_file)
	 index = index + 1
      end
   end
end
