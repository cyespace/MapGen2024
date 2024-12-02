
Files = 
{
	"Script/UI/MenuScene/Image_BackGround.lua",
	"Script/UI/MenuScene/Label_Seed.lua",
	"Script/UI/MenuScene/Label_Title.lua",
	"Script/UI/MenuScene/Label_Version.lua",
	"Script/UI/MenuScene/TextInput_Seed.lua"
}

function LoadUIElements()
	for _, filePath in ipairs(Files) do
		CPP_LoadUIElement(filePath)
		--print("Load"..filePath.."\n")
	end
end

    