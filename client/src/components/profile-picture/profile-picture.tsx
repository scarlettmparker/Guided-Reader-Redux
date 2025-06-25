import React, { CSSProperties } from "react";
import styles from "./profile-picture.module.css";

interface ProfilePictureProps {
  className?: string;
  avatar?: string;
  discord_id?: string;
  style?: CSSProperties;
  variant?: "default" | "large";
}

const ProfilePicture: React.FC<ProfilePictureProps> = ({
  className = "",
  avatar,
  discord_id,
  style,
  variant = "default",
}) => {
  const avatarUrl =
    avatar && discord_id
      ? `https://cdn.discordapp.com/avatars/${discord_id}/${avatar}?size=${256}`
      : "assets/picture/default_pfp.png";

  const size = variant === "large" ? 256 : 30;
  const variantClass =
    variant === "large" ? styles.profile_picture_large : styles.profile_picture;

  return (
    <img
      src={avatarUrl}
      alt="Discord Avatar"
      className={`${variantClass} ${className}`}
      width={size}
      height={size}
      style={style && style}
    />
  );
};

export default ProfilePicture;
