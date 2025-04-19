import { Component } from "solid-js";
import styles from "./profile-picture.module.css";

interface ProfilePictureProps {
  class?: string;
  avatar?: string;
  discord_id?: string;
}

const ProfilePicture: Component<ProfilePictureProps> = (props) => {
  const { class: class_ = "", avatar, discord_id } = props;

  const avatarUrl =
    avatar && discord_id
      ? `https://cdn.discordapp.com/avatars/${discord_id}/${avatar}`
      : "assets/picture/default_pfp.png";

  return (
    <img
      src={avatarUrl}
      alt="Discord Avatar"
      class={`${styles.profile_picture} ${class_}`}
      width="30"
      height="30"
    />
  );
};

export default ProfilePicture;
