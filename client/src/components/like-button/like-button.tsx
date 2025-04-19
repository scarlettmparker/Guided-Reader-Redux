import { Component, createSignal } from 'solid-js';
import styles from './like-button.module.css';

interface LikeButtonProps {
  class?: string;
  reverse?: boolean;
};

const LikeButton: Component<LikeButtonProps> = (props) => {
  const [active, setActive] = createSignal(false);
  const { class: class_ = "", reverse = false, } = props;

  const handleClick = () => {
    setActive(!active());
    // Perform API call
  };

  return (
    <img
      src={active() ? "/assets/annotation/icons/voted.png" : "/assets/annotation/icons/unvoted.png"}
      alt="Like"
      class={`${styles.like_button} ${class_}`}
      style={{ transform: reverse ? "rotate(180deg)" : "none" }}
      width="20"
      height="20"
      onClick={handleClick}
    />
  );
};

export default LikeButton;