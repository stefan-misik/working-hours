--------------------------------------------------------------------------------
-- Function used to calculate working hours
function Calculate(Arrival, Now, PauseTime)
    -- Get the value representations in minutes
    local MinutesNow = Now.Minute + (60 * Now.Hour)
    local Minutes = MinutesNow - (Arrival.Minute + (60 * Arrival.Hour))

    -- Subtract 30 minutes lunch break after 6 hours
    if (Minutes > (6*60 + 30)) then
        Minutes = Minutes - 30
    elseif (Minutes > (6*60)) then
        Minutes = 6*60
    end

    -- Subtract pause time
    Minutes = Minutes - PauseTime

    -- Make sure the time spent working is not negative
    if (Minutes < 0) then
        Minutes = 0
    end

    -- Assign timer color
    local Color
    if (MinutesNow < (14*60 + 30)) then
        -- RED - Before end of obligatory period (until 14:30)
        Color = 0xed1c24
    elseif (Minutes < (8*60)) then
        -- ORANGE - After end of obligatory period, before 8 hours of work
        Color = 0xff7f27
    else
        -- GREEN - After 8 hours of work and after obligatory period
        Color = 0x22b14c
    end

    -- Calculate worked time
    local Worked = {
        Hour = (math.floor(Minutes / 60)),
        Minute = (Minutes % 60)
    }

    ---- Print debug information
    --print(string.format("Worked for: %i:%i", Worked.Hour, Worked.Minute))

    -- Return calculated working hours and timer color
    return Worked, Color
end

--------------------------------------------------------------------------------
-- Function used to calculate leave time
function LeaveTime(Arrival, PauseTime)
    local Minutes = Arrival.Minute + (60 * Arrival.Hour)

    -- Leave in 8 hours, 30 minutes for launch break
    -- fix whole day overflow
    Minutes = (Minutes + (8*60) + 30) % (24*60)

    -- Add pause time
    Minutes = Minutes + PauseTime

    -- Calculate leave time
    local Leave = {
        Hour = (math.floor(Minutes / 60)),
        Minute = (Minutes % 60)
    }

    ---- Print debug information
    --print(string.format("Leave Time: %i:%i", Leave.Hour, Leave.Minute))

    -- Return the leave time
    return Leave
end
