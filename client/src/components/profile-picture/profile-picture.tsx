import React from "react";
import styles from "./profile-picture.module.css";

interface ProfilePictureProps {
  className?: string;
  avatar?: string;
  discord_id?: string;
}

const ProfilePicture: React.FC<ProfilePictureProps> = ({
  className = "",
  avatar,
  discord_id,
}) => {
  const avatarUrl =
    avatar && discord_id
      ? `https://cdn.discordapp.com/avatars/${discord_id}/${avatar}`
      : "assets/picture/default_pfp.png";

  return (
    <img
      src={avatarUrl}
      alt="Discord Avatar"
      className={`${styles.profile_picture} ${className}`}
      width="30"
      height="30"
    />
  );
};

export default ProfilePicture;
