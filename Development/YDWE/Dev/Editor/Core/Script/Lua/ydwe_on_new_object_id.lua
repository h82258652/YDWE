require "mapanalyzer"
require "filesystem"
require "util"
require "localization"
require "interface_storm"
local ffi = require "ffi"

local object = {}

function object:initialize ()
	self.interface = interface_stormlib()
	self.interface:open_archive(fs.ydwe_path() / "share" / "mpq" / "units.mpq")
	self.manager = mapanalyzer.manager2(self.interface)	
	self.object_type = {
		[0] = mapanalyzer.OBJECT_TYPE.UNIT,
		[1] = mapanalyzer.OBJECT_TYPE.ITEM,
		[2] = mapanalyzer.OBJECT_TYPE.DESTRUCTABLE,
		[3] = mapanalyzer.OBJECT_TYPE.DOODAD,
		[4] = mapanalyzer.OBJECT_TYPE.ABILITY,
		[5] = mapanalyzer.OBJECT_TYPE.BUFF,
		[6] = mapanalyzer.OBJECT_TYPE.UPGRADE,
	}
end
			
function object:original_has (this_, id_string_)
	local this_ptr_ = ffi.cast('uint32_t*', this_)
	local ptr  = this_ptr_[7] + 4
	local size = this_ptr_[6]

	for i = 0, size-2 do
		local id = ffi.cast('uint32_t*', ptr)[0]
		if sys.object_id_int_to_string(id) == id_string_ then
			return true
		end
		ptr = ptr + 24
	end

	return false
end

function object:custom_has (type_, id_string_)
	if not self.object_type[type_] then
		return false
	end
	local table_ = self.manager:load(self.object_type[type_])
	if not table_:get(id_string_) then
		return false
	end
	return true
end

object:initialize()



-- 在新建物体的时候调用，本函数根据用户的操作决定新ID值
-- object_type - 整数类型，物体的类型
-- default_id - 整数类型，系统生成的ID
-- 返回值：新建物体的最终ID，必须是整数类型
event.register(event.EVENT_NEW_OBJECT_ID, false, function (event_data)
	log.debug("**************** on new object id start ****************")	
		
	local object_type = event_data.object_type
	local default_id = event_data.default_id
	-- 刷新配置数据
	global_config:reload()

	-- 如果没有选择手动输入则直接返回
	if global_config:get_integer("FeatureToggle.EnableManualNewId", 0) == 0 then
		log.trace("Disable.")
		return default_id
	end
	
	-- 获取当前窗口
	local foregroundWindow = sys.get_foreground_window()

	-- 循环直到输入合法或者放弃
	while true do
		-- 打开对话框让用户输入
		local ok, id_string = gui.prompt_for_input(
			foregroundWindow, 														-- 父窗口句柄
			_("New Object Id"),														-- 标题栏
			_("Please input new object ID, or cancel to use the default one."),		-- 提示语句
			sys.object_id_int_to_string(default_id),								-- 文本编辑区初始文字
			_("OK"),																-- “确定”按钮文本
			_("Cancel")																-- “取消"按钮文本
		)
		
		-- 用户点了确定，验证输入是否合法。如果点了取消，使用系统默认
		if not ok then
			log.trace("User cancel.")
			return default_id
		end
		
		-- 检查输入是否合法（字符串长度是否为4）
		if #id_string ~= 4 then
			log.trace("User input error(" .. tostring(id_string) .. ").")	
			-- 提示错误
			gui.message_dialog(
				foregroundWindow,
				_("You have entered an invalid ID. The ID must contain just 4 letters or digits. It cannot contain chars other than those in ASCII."),
				_("YDWE"),
				bit.bor(gui.MB_ICONERROR, gui.MB_OK)
			)
		elseif object:custom_has(object_type, id_string) or object:original_has(event_data.class, id_string) then
			log.trace("User input error(" .. tostring(id_string) .. ").")	
			-- 提示错误
			gui.message_dialog(
				foregroundWindow,
				_("You have entered an invalid ID. This ID already exists."),
				_("YDWE"),
				bit.bor(gui.MB_ICONERROR, gui.MB_OK)
			)
		else
			-- 合法，转换为整数返回	
			log.trace("Result " .. tostring(id_string))	
			return sys.object_id_string_to_int(id_string)
		end
				
	end
	return 0
end)
