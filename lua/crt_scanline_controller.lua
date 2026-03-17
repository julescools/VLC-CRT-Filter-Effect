--[[
    CRT Scanline Controller
    VLC Lua Extension for live adjustment of the CRT Scanline video filter.

    Install: Copy to C:\Program Files\VideoLAN\VLC\lua\extensions\
    Access:  VLC menu > View > CRT Scanline Controller

    Requires the CRT Scanline video filter plugin (libcrt_scanline_plugin.dll)
    to be installed and enabled in VLC preferences.
--]]

function descriptor()
    return {
        title = "CRT Scanline Controller",
        version = "1.0",
        author = "Jules Lazaro",
        description = "Live control of CRT scanline effect."
    }
end

-- Current values
local darkness = 35
local spacing = 2
local blend = true
local saved_darkness = 35

-- Dialog widgets
local dlg = nil
local darkness_label = nil
local spacing_label = nil

function activate()
    -- Read current config values
    darkness = vlc.config.get("crtscanline-darkness") or 35
    spacing = vlc.config.get("crtscanline-spacing") or 2
    saved_darkness = darkness

    local blend_val = vlc.config.get("crtscanline-blend")
    if blend_val == nil then
        blend = true
    else
        blend = blend_val
    end

    create_dialog()
end

function deactivate()
    -- Turn off the effect when deactivating
    darkness = 0
    vlc.config.set("crtscanline-darkness", 0)

    if dlg then
        dlg:delete()
        dlg = nil
    end
end

function close()
    -- Called when user closes the dialog window via X button.
    -- Keep the effect running, just close the UI.
    if dlg then
        dlg:delete()
        dlg = nil
    end
end

function create_dialog()
    if dlg then
        dlg:delete()
    end

    dlg = vlc.dialog("CRT Scanline Controller")

    -- === Enable / Disable toggle ===
    dlg:add_label("<b>CRT Scanline Effect</b>", 1, 1, 2, 1)
    dlg:add_button("ON", click_enable, 3, 1, 1, 1)
    dlg:add_button("OFF", click_disable, 4, 1, 1, 1)

    -- === Darkness control ===
    dlg:add_label("Darkness:", 1, 2, 1, 1)
    darkness_label = dlg:add_label("<b>" .. tostring(darkness) .. "</b>", 4, 2, 1, 1)
    dlg:add_button("  −  ", function() adjust_darkness(-5) end, 2, 2, 1, 1)
    dlg:add_button("  +  ", function() adjust_darkness(5) end, 3, 2, 1, 1)

    -- === Spacing control ===
    dlg:add_label("Spacing:", 1, 3, 1, 1)
    spacing_label = dlg:add_label("<b>" .. tostring(spacing) .. "</b>", 4, 3, 1, 1)
    dlg:add_button("  −  ", function() adjust_spacing(-1) end, 2, 3, 1, 1)
    dlg:add_button("  +  ", function() adjust_spacing(1) end, 3, 3, 1, 1)

    -- === Blend mode toggle ===
    dlg:add_label("Blend:", 1, 4, 1, 1)
    dlg:add_button("Smooth", click_blend_on, 2, 4, 1, 1)
    dlg:add_button("Hard", click_blend_off, 3, 4, 1, 1)

    -- === Presets ===
    dlg:add_label("<b>Presets:</b>", 1, 5, 1, 1)
    dlg:add_button("Subtle", click_preset_subtle, 2, 5, 1, 1)
    dlg:add_button("Classic", click_preset_classic, 3, 5, 1, 1)
    dlg:add_button("Heavy", click_preset_heavy, 4, 5, 1, 1)

    -- === Info + Credits ===
    dlg:add_label("<i>Darkness: 0=off, 100=max  |  Spacing: pixels at 480p</i>",
                  1, 6, 4, 1)
    dlg:add_label("Created by Jules Lazaro", 1, 7, 4, 1)
end

-- === Parameter adjustment functions ===

function adjust_darkness(delta)
    darkness = darkness + delta
    if darkness < 0 then darkness = 0 end
    if darkness > 100 then darkness = 100 end
    apply_darkness()
end

function adjust_spacing(delta)
    spacing = spacing + delta
    if spacing < 1 then spacing = 1 end
    if spacing > 20 then spacing = 20 end
    apply_spacing()
end

function apply_darkness()
    vlc.config.set("crtscanline-darkness", darkness)
    if darkness_label then
        darkness_label:set_text("<b>" .. tostring(darkness) .. "</b>")
    end
end

function apply_spacing()
    vlc.config.set("crtscanline-spacing", spacing)
    if spacing_label then
        spacing_label:set_text("<b>" .. tostring(spacing) .. "</b>")
    end
end

function apply_blend()
    vlc.config.set("crtscanline-blend", blend)
end

-- === Button callbacks ===

function click_enable()
    if saved_darkness > 0 then
        darkness = saved_darkness
    else
        darkness = 35
    end
    apply_darkness()
end

function click_disable()
    if darkness > 0 then
        saved_darkness = darkness
    end
    darkness = 0
    apply_darkness()
end

function click_blend_on()
    blend = true
    apply_blend()
end

function click_blend_off()
    blend = false
    apply_blend()
end

-- === Presets ===

function click_preset_subtle()
    darkness = 20
    spacing = 2
    blend = true
    apply_darkness()
    apply_spacing()
    apply_blend()
end

function click_preset_classic()
    darkness = 35
    spacing = 2
    blend = true
    apply_darkness()
    apply_spacing()
    apply_blend()
end

function click_preset_heavy()
    darkness = 60
    spacing = 3
    blend = false
    apply_darkness()
    apply_spacing()
    apply_blend()
end
