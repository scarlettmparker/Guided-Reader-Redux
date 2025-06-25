import React from "react";
import styles from "./profile-picture.module.css";

interface ProfilePictureProps {
  className?: string;
  avatar?: string;
  discord_id?: string;
  variant?: "default" | "large";
}

const ProfilePicture: React.FC<ProfilePictureProps> = ({
  className = "",
  avatar,
  discord_id,
  variant = "default",
}) => {
  const avatarUrl =
    avatar && discord_id
      ? `https://cdn.discordapp.com/avatars/${discord_id}/${avatar}`
      : "assets/picture/default_pfp.png";

  const size = variant === "large" ? 128 : 30;
  const variantClass =
    variant === "large" ? styles.profile_picture_large : styles.profile_picture;

  return (
    <img
      src={avatarUrl}
      alt="Discord Avatar"
      className={`${variantClass} ${className}`}
      width={size}
      height={size}
    />
  );
};

export default ProfilePicture;
