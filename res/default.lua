--------------------------------------------------------------------------------
-- Function used to calculate working hours
function Calculate(Arrival, Now)
    -- Get the value representations in minutes
    local MinutesNow = Now.Minute + (60 * Now.Hour)
    local Minutes = MinutesNow - (Arrival.Minute + (60 * Arrival.Hour))
    
    -- Subtract 5 minutes form beginning
    Minutes = Minutes - 5;

    -- Subtract 30 minutes lunch break after 6 hours
    if (Minutes > (6*60 + 30)) then
        Minutes = Minutes - 30;
    elseif (Minutes > (6*60)) then
        Minutes = 6*60
    end

    -- Subtract 5 minutes from end
    Minutes = Minutes - 5;
    
    -- Make sure the time spent working is not negative
    if (Minutes < 0) then
        Minutes = 0
    end

    --Assign timer color
    local Color
    if (MinutesNow < (14*60 + 30)) then
        -- RED - Before end of obligatory period (until 14:30)
        Color = WhRgb(237, 28, 36)
    elseif (Minutes < (8*60)) then
        -- ORANGE - After end of obligatory period, before 8 hours of work
        Color = WhRgb(255, 127, 39)
    else
        -- GREEN - After 8 hours of work and after obligatory period
        Color = WhRgb(34, 177, 76)
    end

    -- Calculate worked time
    local Worked = WhNewTime(WhFloor(Minutes / 60), Minutes % 60)

    -- Return calculated working hours and timer color
    return Worked, Color
end

--------------------------------------------------------------------------------
-- Function used to calculate leave time
function LeaveTime(Arrival)
    local Minutes = Arrival.Minute + (60 * Arrival.Hour)

    -- Leave in 8 hours, 30 minutes for launch break, and 5 minutes for arrival
    -- and 5 for leave; fix whole day overflow
    Minutes = (Minutes + (8*60) + 30 + 5 + 5) % (24*60)

    -- Calculate leave time
    local Leave = WhNewTime(WhFloor(Minutes / 60), Minutes % 60)

    return Leave
end
