BiomeSetting = 
{
	--height / temperature / biome index
	-- deep water
	{0.2, 0.02, 1},
	{0.2, 0.05, 2},
	{0.2, 1.0,  3},
	
	-- shallow water
	{0.3, 0.02, 1},
	{0.3, 0.05, 2},
	{0.3, 1.0,  4},
	
	-- coast line 
	{0.4, 0.02, 1},
	{0.4, 0.05, 2},
	{0.4, 1.0, 	5},
	
	-- beach
	{0.45, 0.05,1},
	{0.45, 1.0, 6},
	
	-- lowland
	{0.6, 0.05, 10},
	{0.6, 0.15, 13},
	{0.6, 0.35, 12},
	{0.6, 0.45, 11},
	{0.6, 0.55, 15},
	{0.6, 0.65, 16},
	{0.6, 1.0, 17},
	
	-- midland
	{0.75, 0.1, 10},
	{0.75, 0.3, 9},
	{0.75, 0.5, 8},
	{0.75, 0.7, 7},
	{0.75, 0.8, 15},
	{0.75, 1.0, 16},
	
	-- highland
	{0.85, 0.15, 21},
	{0.85, 0.35, 18},
	{0.85, 0.55, 19},
	{0.85, 1.0, 14},
	
	-- mountain
	{0.95, 0.4, 22},
	{0.95, 0.5, 21},
	{0.95, 0.8, 19},
	{0.95, 1.0, 18},
	
	{1.0, 1.0, 22}
}

BiomeData = 
{
	-- color / name
	{{255,255,255}, "IceCap"}, 				--1
	{{190,245,240},	"ThinIce"}, 			--2
	{{15,15,100},	"DeepWater"}, 			--3
	{{25,50,135},	"ShallowWater"}, 		--4
	{{55,105,200},	"CoastLine"}, 			--5
	
	{{240,240,120},	"Beach"}, 				--6
	
	{{90,150,55},	"GrassSavanna"}, 		--7		hot grassland, with some trees
	{{120,165,65},   "Prairie"}, 			--8		tall grassland
	{{150,170,95},   "GrassSteppe"}, 		--9		short grassland
	{{150,185,165},	"Tundra"}, 				--10
	
	{{30,80,10},	"TropicalForest"}, 		--11
	{{30,110,15},	"TemperateForest"},		--12
	{{50,90,75},	"Taiga"}, 				--13	aka boreal forest
	{{90,110,100},  "MontaneForest"}, 		--14
	
	{{180,165,45},	"XericShrubland"}, 		--15
	{{220,155,30},  "DrySteppe"}, 			--16
	{{245,220,30}, 	"Desert"}, 				--17
	
	{{115,100,95},  "Rocks"}, 				--18
	{{95,80,75},  "BarrenHills"}, 			--19
	{{75,60,55},  "Mountain"}, 				--20
	
	{{190,245,240}, "FrozenLand"},			--21
	{{255,255,255}, "SnowCoveredLand"},		--22
	
	{{255,0,0},  "Test"} 					--99
}

function GetBiomeData(height,temperature)
	for i, biomeValue in ipairs(BiomeSetting) do
		if(height <= biomeValue[1] and temperature <= biomeValue[2] ) then
			biomeIndex = biomeValue[3]
			return  biomeIndex,
					BiomeData[biomeIndex][1][1], --r
					BiomeData[biomeIndex][1][2], --g
					BiomeData[biomeIndex][1][3]  --b
		
		end
	end
	print("Error: in height = ",height," in temp = ", temperature )
	return -1,0,0,0
end
