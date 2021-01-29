FROM ubuntu:xenial

ARG username
ARG uid
ARG gid
ARG rosdistro=kinetic

USER root

# Set computer clock
RUN rm /etc/localtime
RUN ln -s /usr/share/zoneinfo/Japan /etc/localtime

# Update user permissions
RUN apt-get update --fix-missing && apt-get install -y sudo apt-utils
RUN export uid=${uid} gid=${gid} && \
    echo "${username}:x:${uid}:${gid}:${username},,,:/home/${username}:/bin/bash" >> /etc/passwd && \
    echo "${username}:x:${uid}:" >> /etc/group
RUN echo "${username} ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/${username}
RUN chmod 0440 /etc/sudoers.d/${username}
RUN mkdir -p /home/${username} && \
    chown ${uid}:${gid} -R /home/${username}

# INSTALL LINUX LIBRARIES
RUN apt-get update --fix-missing && \
    apt-get install -y \
    software-properties-common

# Update newly installed libraries
RUN ldconfig

USER ${username}

VOLUME ["/src"]
WORKDIR "/src"
