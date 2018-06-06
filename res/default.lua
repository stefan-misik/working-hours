-- Function to calculate working hours
function Calculate(Arrival, Now)

    -- Get the value representations in minutes
    MinutesNow = Now.Minute + (60 * Now.Hour)
    MinutesArrival = Arrival.Minute + (60 * Arrival.Hour)
    
    -- Calculate the difference
    Minutes = MinutesNow - MinutesArrival
    
    -- Subtract 5 minutes form beginning
    Minutes = Minutes - 5;

    -- Subtract 30 minutes lunch break after 6 hours

    -- Subtract 5 minutes from end
    Minutes = Minutes - 5;
    
    -- Make sure the time spent working is not negative
    if (Minutes < 0) then
        Minutes = 0
    end

    -- Calculate worked time
    Worked = WhNewTime(Minutes / 60, Minutes % 60)

    return Worked, 0
end


-- Function to calculate leave time
function LeaveTime(Arrival)

    return Arrival
end