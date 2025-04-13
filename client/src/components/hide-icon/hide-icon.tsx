import { Component } from "solid-js";
import styles from './hide-icon.module.css';

interface HideIconProps {
  reverse?: boolean;
  class?: string;
};

const HideIcon: Component<HideIconProps> = (props) => {
  const { reverse, class: _class } = props;

  return (
    <div class={`${styles.hide_icon} ${_class}`}>
      {reverse ? '>' : '<'}
    </div>
  );
};

export default HideIcon;