#pragma once

class Game {
  public:
    void build();
    void destroy();

    void rebuild();

    void loop();
    const bool &isRunning();

  private:
    bool m_IsRunning = false;
};
