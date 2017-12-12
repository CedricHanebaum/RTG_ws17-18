
/// Implement the mainLoop variant that is shown on screen
/// You can get the current wall clock via:
///     double secs = timeInSeconds();
/// ============= STUDENT CODE BEGIN =============

auto fixedTimestep = true;
auto variableRendering = false;
auto targetFPS = 25;
auto frameSkip = 5;

if (fixedTimestep && variableRendering)
{
    auto targetTimestep = 1.0 / targetFPS;
    auto nextTime = timeInSeconds();

    while (!shouldClose())
    {
        // fixed timestep
        while (timeInSeconds() > nextTime)
        {
            // accumulate time
            nextTime += targetTimestep;

            // run simulation with fixed timestep
            update(targetTimestep);
            ++totalFrames; // we only count simulation FPS
        }

        // render with variable timestep
        render();
    }
}
else if (fixedTimestep /* && frameSkip */)
{
    auto targetTimestep = 1.0 / targetFPS;
    auto nextTime = timeInSeconds();

    while (!shouldClose())
    {
        // fixed timestep
        auto framesLeft = frameSkip;
        while (timeInSeconds() > nextTime && framesLeft > 0)
        {
            // accumulate time
            nextTime += targetTimestep;

            // run simulation AND rendering with fixed timestep
            update(targetTimestep);
            render();
            ++totalFrames;
            --framesLeft;

            // comment in to see skipped frames
            // sleepSeconds(0.1);
        }

        if (framesLeft <= 0)
            glow::info() << "skipped some frames";
    }
}
else // variable timestep
{
    auto targetTimestep = 1.0 / targetFPS;
    auto prevTime = 0.0;
    auto currTime = timeInSeconds();

    while (!shouldClose())
    {
        // calculate elapsed time
        prevTime = currTime;
        currTime = timeInSeconds();
        auto dt = currTime - prevTime;

        // perform simulation AND rendering
        update(dt);
        render();
        ++totalFrames;

        // sleep such that dt is expected to be targetTimestep next frame
        auto nextTime = currTime + targetTimestep;
        sleepSeconds(nextTime - timeInSeconds());
    }
}

/// ============= STUDENT CODE END =============