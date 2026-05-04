# Base image: Ubuntu 24.04 (Noble Numbat)
FROM ubuntu:24.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update system and install essential C++ build tools and utilities
# Consolidated into one layer to optimize caching and reduce image size
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    gdb \
    git \
    cppcheck \
    clang-tidy \
    curl \
    zip \
    unzip \
    tar \
    python3 \
    python3-pip \
    pipx \
    nodejs \
    npm \
    gh \
    sudo \
    && rm -rf /var/lib/apt/lists/*

# Install global tools as root
RUN npm install -g @github/copilot

# Ubuntu 24.04 already has a user named 'ubuntu' with UID 1000.
ARG USERNAME=ubuntu

# Setup sudo for the existing ubuntu user
RUN echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

ENV PATH="/home/ubuntu/.local/bin:${PATH}"

# Set the default user
USER $USERNAME

# Install aider-chat via pipx and add it to PATH
RUN pipx install aider-chat && pipx ensurepath

# Add aliases for Gemini and Copilot to .bashrc
# Placed at the end to avoid invalidating cache for previous layers during edits
RUN echo 'alias aider-gen="env -u GOOGLE_APPLICATION_CREDENTIALS -u GOOGLE_CLOUD_PROJECT aider --model gemini/gemini-2.0-pro-exp"' >> /home/ubuntu/.bashrc && \
    echo 'alias aider-flash="env -u GOOGLE_APPLICATION_CREDENTIALS -u GOOGLE_CLOUD_PROJECT aider --model gemini/gemini-2.0-flash"' >> /home/ubuntu/.bashrc && \
    echo 'alias aider-lite="env -u GOOGLE_APPLICATION_CREDENTIALS -u GOOGLE_CLOUD_PROJECT aider --model gemini/gemini-2.0-flash-lite"' >> /home/ubuntu/.bashrc && \
    echo 'alias aider-pro="aider --model github_copilot/claude-opus-4.6-fast"' >> /home/ubuntu/.bashrc && \
    echo 'alias aider-sonnet="aider --model github_copilot/claude-sonnet-4.5"' >> /home/ubuntu/.bashrc && \
    echo 'alias aider-gpt="aider --model github_copilot/gpt-4o"' >> /home/ubuntu/.bashrc

# Set the default working directory
WORKDIR /workspace